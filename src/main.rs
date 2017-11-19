#[macro_use]
extern crate gfx;
extern crate gfx_window_glutin;
extern crate glutin;
extern crate rand;

use gfx::traits::FactoryExt;
use gfx::{Device, Factory};
use gfx_window_glutin as gfx_glutin;
use glutin::{GlContext, GlRequest};
use glutin::Api::OpenGl;
use rand::Rng;

pub type ColorFormat = gfx::format::Rgba8;
pub type DepthFormat = gfx::format::DepthStencil;

const BLACK: [f32; 4] = [0.0, 0.0, 0.0, 1.0];

gfx_defines!{
    vertex Vertex {
        pos: [f32; 2] = "a_Pos",
        tex_coord: [f32; 2] = "a_TexCoord",
    }
    pipeline pipe {
        vbuf: gfx::VertexBuffer<Vertex> = (),
        tex: gfx::TextureSampler<[f32; 4]> = "t_Texture",
        out: gfx::RenderTarget<ColorFormat> = "Target0",
    }
}

fn gen_texture<F, R>(factory: &mut F, window: &glutin::Window, pixels: &mut Vec<u8>) -> gfx::handle::ShaderResourceView<R, [f32; 4]>
    where F: gfx::Factory<R>, R: gfx::Resources
{
    use gfx::format::Rgba8;
    let mut rng = rand::thread_rng();
    let (width, height) = window.get_inner_size_pixels().unwrap_or((640, 480));
    pixels.resize((width * height * 4) as usize, 0);
    rng.fill_bytes(pixels);
    let kind = gfx::texture::Kind::D2(width as u16, height as u16, gfx::texture::AaMode::Single);
    let (_, view) = factory.create_texture_immutable_u8::<Rgba8>(kind, &[&pixels]).unwrap();
    view
}

fn main() {
    let mut events = glutin::EventsLoop::new();
    let windowbuilder = glutin::WindowBuilder::new()
        .with_title("Pixel scrambler".to_string())
        .with_fullscreen(Some(events.get_primary_monitor()));
    let contextbuilder = glutin::ContextBuilder::new()
        .with_gl(GlRequest::Specific(OpenGl, (3, 2)))
        .with_vsync(true);
    let (window, mut device, mut factory, color_view, _) =
        gfx_glutin::init::<ColorFormat, DepthFormat>(windowbuilder, contextbuilder, &events);

    let pso = factory.create_pipeline_simple(
        include_bytes!("shader/default.glslv"),
        include_bytes!("shader/default.glslf"),
        pipe::new()
    ).unwrap();
    let mut encoder: gfx::Encoder<_, _> = factory.create_command_buffer().into();
    const SQUARE: [Vertex; 4] = [
        Vertex { pos: [ -1.0, -1.0 ], tex_coord: [0.0, 0.0] },
        Vertex { pos: [  1.0, -1.0 ], tex_coord: [1.0, 0.0] },
        Vertex { pos: [  1.0,  1.0 ], tex_coord: [1.0, 1.0] },
        Vertex { pos: [ -1.0,  1.0 ], tex_coord: [0.0, 1.0] },
    ];
    const INDEX_DATA: &[u16] = &[
        0, 1, 2,
        2, 3, 0
    ];

    let (vertex_buffer, slice) = factory.create_vertex_buffer_with_slice(&SQUARE, INDEX_DATA);
    let mut pixels = Vec::new();
    let sampler = factory.create_sampler(gfx::texture::SamplerInfo::new(gfx::texture::FilterMethod::Scale, gfx::texture::WrapMode::Mirror));
    let texture = gen_texture(&mut factory, &window, &mut pixels);
    let mut data = pipe::Data {
        vbuf: vertex_buffer,
        tex: (texture, sampler),
        out: color_view.clone(),
    };

    let mut run = true;
    while run {
        events.poll_events(|event| {
            if let glutin::Event::WindowEvent { event, .. } = event {
                match event {
                    glutin::WindowEvent::Closed |
                    glutin::WindowEvent::KeyboardInput {
                        input: glutin::KeyboardInput {
                            virtual_keycode: Some(glutin::VirtualKeyCode::Escape), ..
                        }, ..
                    } => run = false,
                    glutin::WindowEvent::Resized(w, h) => {
                        window.context().resize(w, h);
                        println!("Resize event caught: {} {}", w, h);
                    }
                    _ => {},
                }
            }
        });

        data.tex.0 = gen_texture(&mut factory, &window, &mut pixels);

        encoder.clear(&color_view, BLACK);
        encoder.draw(&slice, &pso, &data);
        encoder.flush(&mut device);

        window.swap_buffers().unwrap();
        device.cleanup();
    }
}
