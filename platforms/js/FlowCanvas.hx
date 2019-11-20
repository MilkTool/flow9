import js.Browser;
import js.html.CanvasElement;

import pixi.core.display.Bounds;
import pixi.core.math.shapes.Rectangle;
import pixi.core.display.Container;
import pixi.core.display.DisplayObject;
import pixi.core.math.Matrix;
import pixi.core.renderers.canvas.CanvasRenderer;
import haxe.extern.EitherType;

using DisplayObjectHelper;

class FlowCanvas extends FlowContainer {
	public function new(?worldVisible : Bool = false) {
		super(worldVisible);

		if (RenderSupportJSPixi.RendererType == "html") {
			initNativeWidget('canvas');
			untyped this.isCanvas = true;
		}
	}

	public function updateNativeWidget() {
		if (visible) {
			if (DisplayObjectHelper.DebugUpdate) {
				nativeWidget.setAttribute("update", Std.int(nativeWidget.getAttribute("update")) + 1);
				if (untyped this.from) {
					nativeWidget.setAttribute("from", untyped this.from);
					untyped this.from = null;
				}

				if (untyped this.info) {
					nativeWidget.setAttribute("info", untyped this.info);
				}
			}

			updateNativeWidgetTransformMatrix();
			updateNativeWidgetOpacity();

			renderToCanvas(nativeWidget, context, worldTransform.clone().invert());

			if (worldTransform.tx < 0 || worldTransform.ty < 0) {
				untyped this.localTransformChanged = true;
			}
		}

		updateNativeWidgetDisplay();
	}

	public override function createNativeWidget(?tagName : String = "canvas") : Void {
		super.createNativeWidget(tagName);

		RenderSupportJSPixi.RendererType = 'canvas';
		PixiWorkarounds.workaroundGetContext();

		context = nativeWidget != null ? nativeWidget.getContext('2d', { alpha : true }) : null;

		if (nativeWidget != null) {
			nativeWidget.onpointermove = function(e) {
				RenderSupportJSPixi.PixiRenderer.plugins.interaction.onPointerMove(e);
				nativeWidget.style.cursor = RenderSupportJSPixi.PixiView.style.cursor;
			};
			nativeWidget.onpointerover = function(e) {
				RenderSupportJSPixi.PixiRenderer.plugins.interaction.onPointerOver(e);
			};
			nativeWidget.onpointerout = function(e) {
				RenderSupportJSPixi.PixiRenderer.plugins.interaction.onPointerOut(e);
			};
			nativeWidget.style.pointerEvents = 'auto';
		}

		RenderSupportJSPixi.RendererType = 'html';
		PixiWorkarounds.workaroundGetContext();
	}

	public override function destroy(?options : EitherType<Bool, DestroyOptions>) : Void {
		super.destroy(options);

		if (context != null) {
			untyped __js__("delete this.context");
		}
	}
}