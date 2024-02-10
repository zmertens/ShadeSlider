using Godot;
using System;

public partial class SlideShader : Control
{
	private ShaderMaterial _color_rect_material;

	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
			ColorRect _color_rect = GetNode<ColorRect>("Camera2D/CanvasLayer/ColorRect");
			_color_rect_material = _color_rect.Material as ShaderMaterial;
			if (_color_rect_material == null) {
				GD.Print("Error getting shader material");
			}
			// mat.SetShaderParameter("base", new Vector4(0.5f, 0.3f, 0.2f, 1.0f));
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		
	}
}
