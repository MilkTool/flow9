#include "Compiler.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>

namespace flow {

Compiler::Compiler(const Ui::FlowConfig& ui, QString file, QString flowdir) : configUi_(ui), type_(DEFAULT), file_(file), flowdir_(flowdir) {
	if (!QFileInfo(file).exists()) {
		throw std::runtime_error("file '" + file.toStdString() + "' doesn't exist");
	}
	if (!QDir(flowdir).exists()) {
		throw std::runtime_error("directory '" + flowdir.toStdString() + "' doesn't exist");
	}
	if (flowdir_.endsWith(QLatin1Char('/'))) {
		flowdir_ = flowdir_.mid(0, flowdir_.length() - 1);
	}
	confFile_ = findConfig(file_);
	config_ = parseConfig(confFile_.absoluteFilePath());
	QString compiler = config_[QLatin1String("flowcompiler")];
	include_ = config_[QLatin1String("include")];
	if (!include_.isEmpty()) {
		include_ += QLatin1String(",");
	}
	include_ += flowdir_ + QLatin1String("/lib");
	if (compiler == QLatin1String("flowc")) {
		type_ = FLOWC1;
	} else if (compiler == QLatin1String("flow")) {
		type_ = FLOW;
	} else if (compiler == QLatin1String("flowc1")) {
		type_ = FLOWC1;
	} else if (compiler == QLatin1String("flowc2")) {
		type_ = FLOWC2;
	}
}

QString Compiler::invocation() const {
	switch (type_) {
	case FLOW:    return QFileInfo(flowdir_ + QLatin1String("/bin/flow")).absoluteFilePath();
	case FLOWC1:  return QFileInfo(flowdir_ + QLatin1String("/bin/flowc1")).absoluteFilePath();
	case FLOWC2:  return QFileInfo(flowdir_ + QLatin1String("/bin/flowc2")).absoluteFilePath();
	default:      return QLatin1String();
	}
}

QStringList Compiler::includeArgs() const {
	if (!include_.isEmpty()) {
		switch (type_) {
		case Compiler::FLOWC1: return QStringList() << QLatin1String("I=") + include_;
		case Compiler::FLOWC2: return QStringList() << QLatin1String("I=") + include_;
		case Compiler::FLOW:  {
			return QStringList() << QLatin1String("-I") << include_;
			/*QStringList ret;
			for (auto inc : include_.split(QLatin1Char(','))) {
				ret << QLatin1String("-I") << inc;
			}
			return ret;*/
		}
		default: return QStringList();
		}
	} else {
		return QStringList();
	}
}

QStringList Compiler::debugArgs(const Target& t) const {
	switch (type_) {
	case Compiler::FLOWC1: return QStringList() << QLatin1String("debug=1");
	case Compiler::FLOWC2: return QStringList() << QLatin1String("debug=1");
	case Compiler::FLOW:   return QStringList() << QLatin1String("--debuginfo") << t.debug();
	default:               return QStringList();
	}
}

QString Compiler::compiler() const {
	switch (type_) {
	case Compiler::FLOWC1: return QLatin1String("flowc1");
	case Compiler::FLOWC2: return QLatin1String("flowc2");
	case Compiler::FLOW:   return QLatin1String("flowc");
	default:               return QLatin1String();
	}
}

QStringList Compiler::targetArgs(const Target& t) const {
	switch (t.type()) {
	case Target::BYTECODE:
		switch (type_) {
		case Compiler::FLOWC1: return QStringList() << QLatin1String("bytecode=") + t.file();
		case Compiler::FLOWC2: return QStringList() << QLatin1String("bytecode=") + t.file();
		case Compiler::FLOW:   return QStringList() << QLatin1String("--bytecode") << t.file();
		default:               return QStringList();
		}
		break;
	case Target::NODEJS:
		switch (type_) {
		case Compiler::FLOWC1: return QStringList() << QLatin1String("es6=") + t.file() << QLatin1String("nodejs=1");
		case Compiler::FLOWC2: return QStringList() << QLatin1String("es6=") + t.file() << QLatin1String("nodejs=1");
		case Compiler::FLOW:   return QStringList() << QLatin1String("--es6") << t.file();
		default:               return QStringList();
		}
		break;
	default: return QStringList();
	}
}

QStringList Compiler::compileArgs(QString prog) const {
	switch (type_) {
	case Compiler::FLOWC1: {
		QStringList ret;
		QStringList opts = configUi_.compilerOptionsLineEdit->text().split(QLatin1Char(' '), QString::SkipEmptyParts);
		ret << opts;
		if (configUi_.compilationTimePhasesCheckBox->checkState() == Qt::Checked) {
			ret << QLatin1String("timephases=1") << prog;
		} else {
			ret << prog;
		}
		return ret;
	}
	case Compiler::FLOWC2: return QStringList() << QLatin1String("bin-dir=") + flowdir_ + QLatin1String("/bin") << prog;
	case Compiler::FLOW:   return QStringList() << QLatin1String("--dontrun") << prog;
	default:               return QStringList();
	}
}

}
