// appgui.h : Defines the entry point for the application.
//
#pragma once

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <unordered_map>
class APPGUI {
#define GCAG APPGUI::Inst()
#define nk_field_size 4096
public:
#pragma pack(1)
struct nk_field {
    char text[nk_field_size];
    int size;
    nk_field() {
        memset(text, 0, sizeof(text));
        size = 0;
    }
};
#pragma pack()
typedef enum cursor_method_type {
    cursor_sync_query = 0,
    cursor_input_message
}cursor_method_type;
typedef int(*appgui_pfn_callback)(void*);
private:
    int fullscreen = GLFW_FALSE;
    struct nk_vec2 cursor_new = { 0 };
    nk_bool enable_vsync = nk_true;
    cursor_method_type cursor_method = cursor_method_type::cursor_sync_query;

    nk_bool swap_clear = nk_false;
    nk_bool swap_finish = nk_true;
    nk_bool swap_occlusion_query = nk_false;
    nk_bool swap_read_pixels = nk_false;
    GLuint occlusion_query = 0;
private:
    void sample_input(GLFWwindow* window)
    {
        float a = .25f; // exponential smoothing factor

        if (cursor_method == cursor_sync_query) {
            double x = 0.0;
            double y = 0.0;
            glfwGetCursorPos(window, &x, &y);
            cursor_new.x = (float)x;
            cursor_new.y = (float)y;
        }

        cursor_vel.x = (cursor_new.x - cursor_pos.x) * a + cursor_vel.x * (1 - a);
        cursor_vel.y = (cursor_new.y - cursor_pos.y) * a + cursor_vel.y * (1 - a);
        cursor_pos = cursor_new;
    }
    void update_vsync()
    {
        glfwSwapInterval((enable_vsync == nk_true) ? 1 : 0);
    }
    void swap_buffers(GLFWwindow* window)
    {
        glfwSwapBuffers(window);

        if (swap_clear)
        {
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (swap_finish)
        {
            glFinish();
        }
        if (swap_occlusion_query)
        {
            GLint occlusion_result = 0;
            if (!occlusion_query)
            {
                glGenQueries(1, &occlusion_query);
            }
            glBeginQuery(GL_SAMPLES_PASSED, occlusion_query);
            glBegin(GL_POINTS);
            glVertex2f(0, 0);
            glEnd();
            glEndQuery(GL_SAMPLES_PASSED);
            glGetQueryObjectiv(occlusion_query, GL_QUERY_RESULT, &occlusion_result);
        }

        if (swap_read_pixels) 
        {
            unsigned char rgba[4] = { 0 };
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        }
    }
    static void error_callback(int error, const char* description)
    {
        fprintf(stderr, "Error: %s\n", description);
    }
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
    {
        GCAG->cursor_new.x = (float)xpos;
        GCAG->cursor_new.y = (float)ypos;
    }
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action != GLFW_PRESS)
        {
            return;
        }
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, 1);
            break;
        }
    }
public:
    struct nk_image img_load(const char* filename)
    {
        int x = 0;
        int y = 0;
        int n = 0;
        GLuint tex = 0;
        unsigned char* data = stbi_load(filename, &x, &y, &n, 0);
        if (!data) {
            //Load inner img
            char stb[] = {
            '\x89','\x50','\x4E','\x47','\x0D','\x0A','\x1A','\x0A','\x00','\x00','\x00','\x0D','\x49','\x48','\x44','\x52',
            '\x00','\x00','\x00','\x08','\x00','\x00','\x00','\x08','\x08','\x06','\x00','\x00','\x00','\xC4','\x0F','\xBE',
            '\x8B','\x00','\x00','\x00','\x09','\x70','\x48','\x59','\x73','\x00','\x00','\x0B','\x13','\x00','\x00','\x0B',
            '\x13','\x01','\x00','\x9A','\x9C','\x18','\x00','\x00','\x00','\x04','\x67','\x41','\x4D','\x41','\x00','\x00',
            '\xB1','\x8E','\x7C','\xFB','\x51','\x93','\x00','\x00','\x00','\x20','\x63','\x48','\x52','\x4D','\x00','\x00',
            '\x7A','\x25','\x00','\x00','\x80','\x83','\x00','\x00','\xF9','\xFF','\x00','\x00','\x80','\xE9','\x00','\x00',
            '\x75','\x30','\x00','\x00','\xEA','\x60','\x00','\x00','\x3A','\x98','\x00','\x00','\x17','\x6F','\x92','\x5F',
            '\xC5','\x46','\x00','\x00','\x00','\x80','\x49','\x44','\x41','\x54','\x78','\xDA','\x4C','\xCE','\xDB','\x6A',
            '\xC3','\x40','\x0C','\x84','\xE1','\x2F','\xED','\xB6','\x90','\xDA','\xDE','\x83','\x64','\x5C','\x9A','\xF7',
            '\x7F','\xCF','\xF4','\x62','\x9D','\x83','\x60','\x40','\xE8','\x9F','\x91','\x74','\xB9','\x73','\xF7','\xA8',
            '\xEB','\x95','\x75','\x61','\xAB','\xB4','\xC6','\x18','\x0A','\xF8','\xF8','\x64','\xF9','\x61','\x59','\x69',
            '\x1B','\x7D','\xD0','\x3B','\x99','\x8A','\xAF','\xEF','\x99','\x5A','\x37','\x6A','\x65','\x74','\x62','\x10',
            '\x79','\x1A','\x5A','\x9D','\x2B','\x47','\xA3','\x0D','\x32','\xA6','\x9E','\x86','\x0C','\x5A','\x67','\xBC',
            '\x83','\x60','\xDF','\xC9','\x50','\x1C','\xC7','\xBC','\x99','\x39','\xE1','\x9E','\x67','\x1F','\x44','\x28',
            '\x8E','\xDF','\x39','\xC8','\x13','\xC6','\x43','\xF3','\xD1','\xE2','\xEF','\xF6','\x32','\x64','\x3C','\x93',
            '\x5A','\xA7','\x55','\xFF','\x03','\x00','\x98','\xAE','\x09','\xEC','\x29','\xEC','\xA5','\x09','\x00','\x00',
            '\x00','\x00','\x49','\x45','\x4E','\x44','\xAE','\x42','\x60','\x82',
            };
            int stbn = sizeof(stb) / sizeof(*stb);
            data = stbi_load_from_memory((const stbi_uc*)stb, stbn, &x, &y, &n, 0);
        }
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return nk_image_id((int)tex);
    }
    void set_font_style(const char* font_file, float font_size = 9.0)
    {
        struct nk_font_atlas *atlas = nullptr;
        nk_glfw3_font_stash_begin(&atlas);
        if (atlas != nullptr && font_file != nullptr && *font_file)
        {
            // Load Fonts: if none of these are loaded a default font will be used
            // Load Cursor: if you uncomment cursor loading please hide the cursor
            struct nk_font* font = nullptr;
            struct nk_font_config cfg = nk_font_config(32.0);
            cfg.range = nk_font_chinese_glyph_ranges();
            font = nk_font_atlas_add_from_file(atlas, font_file, font_size, &cfg);
            if (font != nullptr)
            {
                nk_style_set_font(nk, &font->handle);
            }
        }
        nk_glfw3_font_stash_end();
    }
    void draw_marker(struct nk_command_buffer* canvas, int lead, struct nk_vec2 pos)
    {
        struct nk_color colors[4] = { nk_rgb(255,0,0), nk_rgb(255,255,0), nk_rgb(0,255,0), nk_rgb(0,96,255) };
        struct nk_rect rect = { -5 + pos.x, -5 + pos.y, 10, 10 };
        nk_fill_circle(canvas, rect, colors[lead]);
    }
public:
    std::unordered_map<std::string, struct nk_image> image_list;
    std::unordered_map<std::string, struct nk_field> field_list;
    struct nk_context* nk = nullptr;
    int show_forecasts = nk_true;
    struct nk_vec2 cursor_pos = { 0 };
    struct nk_vec2 cursor_vel = { 0 };
    double frame_rate = 0.0;
    struct nk_rect area = { 0 };
public:
    int run(
        appgui_pfn_callback cb_init, void * pv_init,
        appgui_pfn_callback cb_main, void* pv_main,
        appgui_pfn_callback cb_exit = nullptr, void* pv_exit = nullptr)
    {
        int ch = 0;
        int width = 0;
        int height = 0;
        unsigned long frame_count = 0;
        double last_time = 0.0;
        double current_time = 0.0;
        GLFWmonitor* monitor = nullptr;
        GLFWwindow* window = nullptr;

        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
        {
            return (EXIT_FAILURE);
        }

        if (fullscreen)
        {
            const GLFWvidmode* mode;

            monitor = glfwGetPrimaryMonitor();
            mode = glfwGetVideoMode(monitor);

            width = mode->width;
            height = mode->height;
        }
        else
        {
            width = 640;
            height = 480;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        window = glfwCreateWindow(width, height, "", monitor, NULL);
        if (!window)
        {
            glfwTerminate();
            return (EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        update_vsync();

        last_time = glfwGetTime();

        nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_pos_callback);

        if (cb_init != nullptr)
        {
            cb_init(pv_init);
        }
        while (!glfwWindowShouldClose(window))
        {
            int width = 0;
            int height = 0;

            glfwPollEvents();
            sample_input(window);

            glfwGetWindowSize(window, &width, &height);
            area = nk_rect(0.f, 0.f, (float)width, (float)height);

            glClear(GL_COLOR_BUFFER_BIT);
            nk_glfw3_new_frame();
            if (nk_begin(nk, "", area, 0))
            {
                if (cb_main != nullptr)
                {
                    cb_main(pv_main);
                }
            }

            nk_end(nk);
            nk_glfw3_render(NK_ANTI_ALIASING_ON);

            swap_buffers(window);
            frame_count++;

            current_time = glfwGetTime();
            if (current_time - last_time > 1.0)
            {
                frame_rate = frame_count / (current_time - last_time);
                frame_count = 0;
                last_time = current_time;
            }
#ifndef _MSC_VER
            std::this_thread::sleep_for(std::chrono::microseconds(10000));
#endif // !_MSC_VER
        }
        if (cb_exit != nullptr)
        {
            cb_exit(pv_exit);
        }
        nk_glfw3_shutdown();
        glfwTerminate();
        return (EXIT_SUCCESS);
    }
public:
    static APPGUI* Inst() {
        static APPGUI APPGUIInstance;
        return &APPGUIInstance;
    }
};