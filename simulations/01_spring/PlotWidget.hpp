#pragma once
#include <string>
#include <vector>
#include <imgui.h>

struct PlotSeries {
    std::string name;
    ImU32 color;
    bool visible = true;
    std::vector<ImVec2> points;
    float thickness = 1.5f;

    PlotSeries(const std::string& n, ImU32 c, float th = 1.5f)
        : name(n), color(c), visible(true), thickness(th) {}
};

class PlotWidget {
public:
    std::string title;
    std::vector<PlotSeries> series;

    bool isPhasePlot = false;
    bool showCrosshairs = true;
    bool showLegend = true;

    PlotWidget(const std::string& plotTitle, bool phasePlot = false);

    int addSeries(const std::string& name, ImU32 color, float thickness = 1.5f);
    void render(const ImVec2& size = ImVec2(-1.0f, 220.0f));

private:
    void computeBounds(float& minX, float& maxX, float& minY, float& maxY) const;
};
