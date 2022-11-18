// Slope Defense - app.cpp

#include "app.h"

#include <imgui_internal.h>
#include <implot_internal.h>

#include <exprtk.hpp>

typedef exprtk::symbol_table<value_t> symbol_table_t;
typedef exprtk::expression<value_t>   expression_t;
typedef exprtk::parser<value_t>       parser_t;

template <typename T>
inline T random_range(T min, T max) {
    T scale = rand() / (T) RAND_MAX;
    return min + scale * ( max - min );
}


MyApp::MyApp() {
    reset_game();
}

void MyApp::reset_game() {
    gameState.score = 0;
    gameState.wave = 1;
    gameState.things.clear();
    gameState.firing_counter = 0;
    gameState.plot_min = -10;
    gameState.plot_max = 10;
    start_wave();
}

//    const int num_pts = 1000;
#define num_pts 1000
static ImPlotPoint pts[num_pts];
static bool is_valid = false;

static bool show_demo = false;
static bool show_implot_demo = false;
static bool show_style_editor = false;

static bool set_focus = true;

void MyApp::calculate() {
    value_t val_x;
    value_t val_t;
    symbol_table_t symbol_table;
    expression_t expression;
    parser_t parser;

//        symbol_table.clear();
    symbol_table.add_variable("x", val_x);
    symbol_table.add_variable("t", val_t);
    symbol_table.add_constants();
    expression.register_symbol_table(symbol_table);

    is_valid = parser.compile(gameState.formula, expression);

    if (is_valid) {
        val_t = GetTime();
        for (int i=0; i<num_pts; i++) {
            val_x = i/(value_t)(num_pts-1) * (gameState.plot_max - gameState.plot_min) + gameState.plot_min;
            pts[i].x = val_x;
            pts[i].y = expression.value();
        }

        // check for collisions
        for (auto& t : gameState.things) {

            val_x = t.pos.x;
            value_t val_y = expression.value();

            t.distance = abs(val_y - t.pos.y);

            // already popping?
            if (t.pop_counter > 0) continue;

            if (gameState.firing_counter > 0) {
                if (t.distance < t.radius * gameState.grace_factor) {
                    t.pop_counter = 0.3;
                }
            }
        }
    }
}

void MyApp::start_wave() {
    for (int i=0; i<gameState.wave * 2 + 5; i++) {
        Thing t;
        t.pos = ImVec2(random_range(gameState.plot_min, gameState.plot_max),
                       random_range(gameState.plot_min, gameState.plot_max));
        t.vel = ImVec2(random_range(-1.0, 1.0),
                       random_range(-1.0, 1.0));
        t.radius = (gameState.plot_max - gameState.plot_min) / 100.0;
        t.color = ImColor(random_range(0.2, 1.0),
                          random_range(0.2, 1.0),
                          random_range(0.2, 1.0),
                          1.0f);
        t.pop_counter = 0;
        gameState.things.push_back(t);
    }
}

void MyApp::update_things() {
    float dt = GetIO().DeltaTime;

    auto it = gameState.things.begin();
    while (it != gameState.things.end()) {

        if (it->pop_counter > 0) {
            it->pop_counter -= dt;
            if (it->pop_counter <= 0) {
                it = gameState.things.erase(it);
                continue;
            }

            it->radius *= 1.1;
            it->color = ImColor(1.0f, 1.0f, 1.0f, 10*ImSaturate(it->pop_counter));
        }

        it->pos.x += it->vel.x * dt * gameState.difficulty;
        it->pos.y += it->vel.y * dt * gameState.difficulty;

        // wrap
        const value_t plot_size = gameState.plot_max - gameState.plot_min;
        if (it->pos.x < gameState.plot_min) it->pos.x += plot_size;
        if (it->pos.y < gameState.plot_min) it->pos.y += plot_size;
        if (it->pos.x > gameState.plot_max) it->pos.x -= plot_size;
        if (it->pos.y > gameState.plot_max) it->pos.y -= plot_size;

        ++it;
    }

    // Update the graph based on the current formula
    // Note: We do this every frame beacause it might vary based on time 't'
    // and also because collisions are handled in there
    calculate();

    if (gameState.firing_counter > 0) {
        gameState.firing_counter = fmaxf(0, gameState.firing_counter - dt);
    };

    if (gameState.things.size() == 0) {
        gameState.wave++;
        start_wave();
    }
}

void MyApp::main_menu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Style Editor", NULL, &show_style_editor);
            ImGui::MenuItem("ImGui Demo", NULL, &show_demo);
            ImGui::MenuItem("ImPlot Demo", NULL, &show_implot_demo);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static int MyCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        MyApp* app = (MyApp*)data->UserData;
        app->fire();
    }
    return 0;
}

void MyApp::fire() {
    gameState.firing_counter = 3;
    set_focus = true;
}

void MyApp::game_grid(ImVec2 grid_size) {

    ImPlot::BeginPlot("Game Grid", 
                      grid_size,
                      0);
    ImPlot::SetupAxis(ImAxis_X1, "x",
                      ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoHighlight);
    ImPlot::SetupAxis(ImAxis_Y1, "y",
                      ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoHighlight);
    ImPlot::SetupAxisLimits(ImAxis_X1,
                            gameState.plot_min, gameState.plot_max,
                            ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1,
                            gameState.plot_min, gameState.plot_max,
                            ImGuiCond_Always);

    const ImVec4 line_normal_color = (ImVec4)ImColor(0.3f, 0.3f, 0.8f, 1.0f);
    const ImVec4 line_firing_color = (ImVec4)ImColor(1.0f, 0.7f, 0.1f, 1.0f);
    ImVec4 line_color = ImLerp(line_normal_color,
                               line_firing_color,
                               ImSaturate(gameState.firing_counter));
    float line_weight = ImLerp(1.0f, 5.0f, ImSaturate(gameState.firing_counter));

    if (is_valid) {
        ImPlot::SetNextLineStyle(line_color, line_weight);
        ImPlot::PlotLine("", &pts[0].x, &pts[0].y, num_pts, 0, 0, sizeof(ImPlotPoint));
    }

    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
    for (int i=0; i<gameState.things.size(); i++) {
        auto& thing = gameState.things[i];
        auto x_scale = ImPlot::GetCurrentPlot()->XAxis(0).ScaleToPixel;
        double pixel_radius = thing.radius * x_scale;
        char name[100];
        snprintf(name, sizeof(name), "P%d", i);
        double x = thing.pos.x;
        double y = thing.pos.y;
        value_t plot_height = gameState.plot_max - gameState.plot_min;
        if (ImPlot::DragPoint(i, &x, &y, thing.color, pixel_radius)) {
            if (gameState.allow_dragging) {
                thing.pos.x = x;
                thing.pos.y = y;
            }
        }
    }

    ImPlot::EndPlot();
}

void MyApp::controls() {
    ImGui::BeginGroup();

    ImGui::Text("Formula");

    ImGui::Text("y = ");
    ImGui::SameLine();
    char buf[1000];
    snprintf(buf, sizeof(buf), "%s", gameState.formula.c_str());
    if (set_focus) {
        ImGui::SetKeyboardFocusHere();
        set_focus = false;
    }
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##Formula", buf, sizeof(buf), ImGuiInputTextFlags_CallbackCompletion, MyCallback, this)) {
        gameState.formula = string(buf);
    }
    ImGui::SetItemDefaultFocus();
    if (ImGui::IsItemDeactivatedAfterEdit()) {
//            fire();
    }

    ImGui::TextDisabled(is_valid ? "" : "Invalid formula.");

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255,150,0,255));
    if (ImGui::Button("Activate!", ImVec2(ImGui::GetContentRegionAvail().x,0))) {
        fire();
    }
    ImGui::PopStyleColor();
    ImGui::TextDisabled("Click or press Tab");

    ImGui::TextUnformatted("");  // spacer

    ImGui::Text("Wave");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputInt("##Wave", &gameState.wave)) {
        gameState.wave = max(1, gameState.wave);
        start_wave();
    }

    ImGui::Text("Difficulty");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##Difficulty", &gameState.difficulty, 0.01, 0, 10)) {
        ;
    }

    ImGui::EndGroup();
}

void MyApp::game_window() {
    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Slope Defense", NULL,
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoDecoration
                 );

    main_menu();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float size = fminf(avail.x, avail.y);
    ImVec2 grid_size(size, size);

    game_grid(grid_size);

    ImGui::SameLine();

    //        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-500);
    controls();

    ImGui::End();
}

void MyApp::setup() {
//        ImGui::StyleColorsMahiDark1();
    ImGui::StyleColorsClassic();
    ImPlot::StyleColorsClassic();
    ImGuiStyle &style = ImGui::GetStyle();
    // Main
    style.WindowBorderSize = 0;
    style.WindowPadding    = ImVec2(8, 8);
    style.FramePadding     = ImVec2(3, 2);
    style.ItemSpacing      = ImVec2(4, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing    = 20.0f;
    style.ScrollbarSize    = 15.0f;
    style.GrabMinSize      = 5.0f;
    // Rounding
    style.WindowRounding    = 2.0f;
    style.ChildRounding     = 2.0f;
    style.FrameRounding     = 2.0f;
    style.PopupRounding     = 2.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 2.0f;
    // Alignment
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    // Setup Dear ImGui style
    ImGuiIO &io = ImGui::GetIO();

    io.FontGlobalScale *= 1.75f;
}

void MyApp::update() {
    update_things();

    game_window();

    if (show_demo) {
        ImGui::ShowDemoWindow(&show_demo);
    }

    if (show_implot_demo) {
        ImPlot::ShowDemoWindow(&show_implot_demo);
    }

    if (show_style_editor) {
        ImGui::Begin("Style Editor", &show_style_editor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }
}


