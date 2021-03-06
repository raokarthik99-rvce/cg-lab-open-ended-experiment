#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"

Model *model     = NULL;
const int width  = 800;
const int height = 800;

Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);

struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    Vec2f varying_uv[3];
    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        varying_uv[nthvert] = model->uv(iface, nthvert);
        return proj<3>(gl_Vertex / gl_Vertex[3]);                  // project homogeneous coordinates to 3d
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        
        //Include for texture mapping
        color = model->diffuse(varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2])*intensity; //interpolate tex for the current pixel
        
        //Include for grayscale 
        //color = TGAColor(255, 255, 255)*intensity;
        
        return false;                              // no, we do not discard this pixel
    }
};

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head/african_head.obj");
    }

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();

    TGAImage image  (width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model->nfaces(); i++) {
        GouraudShader shader;
        Vec3i screen_coords[3];
        for (int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.  flip_vertically(); // to place the origin in the bottom left corner of the image
    zbuffer.flip_vertically();

    //Include for grayscale
    //image.  write_tga_file("outputGouraudGrayscale.tga");
    
    //Include for texture mapping
    image.  write_tga_file("outputGouraudAttemptatTextureMap.tga");
   
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
