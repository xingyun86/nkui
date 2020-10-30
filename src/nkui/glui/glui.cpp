// glui.cpp : Defines the entry point for the application.
//
#include "glui.h"
#include <appgui.h>

int main(int argc, char** argv)
{
    /*auto s = file_reader("test.png");
    for (size_t i = 0; i < s.size(); i++)
    {
        if (i != 0 && i % 16 == 0)
        {
            printf("\n");
        }
        printf("'\\x%02X',", (uint8_t)s.at(i));
    }
    return 0;*/
    GCAG->run([](void* p)->int
        {
            GCAG->image_list.emplace("home_img", GCAG->img_load(GCUT->home_file.c_str()));
            GCAG->field_list.emplace("message", APPGUI::nk_field());
            GCAG->set_font_style(GCUT->font_file.c_str(), 13.0);
            return 0;
        }, nullptr,
        [](void* p)->int
        {
            auto nk = GCAG->nk;
            auto canvas = nk_window_get_canvas(nk);
            for (int lead = GCAG->show_forecasts ? 3 : 0; lead >= 0; lead--)
            {
                GCAG->draw_marker(canvas, lead, nk_vec2(GCAG->cursor_pos.x + GCAG->cursor_vel.x * lead, GCAG->cursor_pos.y + GCAG->cursor_vel.y * lead));
            }
            nk_layout_row_dynamic(nk, 30, 1);

            GCUT->ShowAPUState([](void* state, void* p)
                {
                    nk_label(GCAG->nk, (const char*)state, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
                    return 0;
                }, nullptr);

            nk_label(nk, u8"IP地址列表:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

            GCUT->ShowIPList([](void* ip_list, void* p)
                {
                    std::vector<std::string>* vIp = (std::vector<std::string>*)ip_list;
                    auto nk = GCAG->nk;
                    auto canvas = nk_window_get_canvas(nk);
                    for (size_t n = 0; n < vIp->size(); n++)
                    {
                        nk_layout_row_begin(nk, NK_STATIC, 12, 2);
                        nk_layout_row_push(nk, 25);
                        GCAG->draw_marker(canvas, (int)n, nk_layout_space_to_screen(nk, nk_vec2(20, 5)));
                        nk_label(nk, "", 0);
                        nk_layout_row_push(nk, 500);
                        nk_labelf(nk, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, "- %d - [%s]", (int)n, vIp->at(n).c_str());
                        nk_layout_row_end(nk);
                    }
                    return 0;
                }, nullptr);

            nk_layout_row_dynamic(nk, 20, 1);

            nk_label(nk, "", 0); // separator
            ////////////////////////////////////////
            /*nk_layout_row_static(nk, 32, 32, 1);
            bool is_clicked = nk_false;
            const struct nk_input* in;
            auto state = nk_widget(&GCAG->area, nk);
            if (!state) return 0;
            in = ((state == NK_WIDGET_ROM) || nk->current->layout->flags & NK_WINDOW_ROM) ? 0 : &nk->input;
            if (nk_do_button_image(&nk->last_widget_state,
                &nk->current->buffer,
                GCAG->area,
                GCAG->image_list["home_img"],
                NK_BUTTON_DEFAULT,
                &nk->style.button,
                in))
            {
                is_clicked = nk_true;
                printf("is_clicked=%d\n", is_clicked);
            }
            nk_layout_row_static(nk, 32, 80, 1);
            nk_label(nk, "input text:", NK_TEXT_LEFT);
            nk_layout_row_static(nk, 32, 160, 1);
            nk_edit_string(nk, NK_EDIT_SIMPLE, GCAG->field_list["message"].text, &GCAG->field_list["message"].size, nk_field_size, nk_filter_default);*/
            ////////////////////////////////////////
            nk_layout_row_static(nk, 72, 72, 1);
            nk_image(nk, GCAG->image_list["home_img"]);
            nk_value_float(nk, "FPS", (float)GCAG->frame_rate);

            return 0;
        }, nullptr);
}