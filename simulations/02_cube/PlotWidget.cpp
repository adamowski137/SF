#include "PlotWidget.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

PlotWidget::PlotWidget(const std::string& plotTitle, bool phasePlot)
    : title(plotTitle), isPhasePlot(phasePlot) {
}

int PlotWidget::addSeries(const std::string& name, ImU32 color, float thickness) {
    series.emplace_back(name, color, thickness);
    return static_cast<int>(series.size()) - 1;
}

void PlotWidget::computeBounds(float& minX, float& maxX, float& minY, float& maxY) const {
    minX = 1e9f;
    maxX = -1e9f;
    minY = 1e9f;
    maxY = -1e9f;

    for (const auto& s : series) {
        if (!s.visible || s.points.empty()) continue;
        for (const auto& pt : s.points) {
            minX = std::min(minX, pt.x);
            maxX = std::max(maxX, pt.x);
            minY = std::min(minY, pt.y);
            maxY = std::max(maxY, pt.y);
        }
    }

    if (minX > maxX) {
        minX = 0.0f;
        maxX = 10.0f;
    } else if (std::abs(maxX - minX) < 1e-4f) {
        minX -= 1.0f;
        maxX += 1.0f;
    }

    if (minY > maxY) {
        minY = -2.0f;
        maxY = 2.0f;
    } else if (std::abs(maxY - minY) < 1e-4f) {
        minY -= 1.0f;
        maxY += 1.0f;
    }

    if (isPhasePlot) {
        float span = std::max({ std::abs(minX), std::abs(maxX), std::abs(minY), std::abs(maxY) });
        span = std::max(1.0f, std::ceil(span * 1.15f));
        minX = -span;
        maxX = span;
        minY = -span;
        maxY = span;
    } else {
        float dy = maxY - minY;
        minY -= dy * 0.08f;
        maxY += dy * 0.08f;
    }
}

void PlotWidget::render(const ImVec2& size) {
    ImVec2 canvasSize = size;
    if (canvasSize.x <= 0.0f) {
        canvasSize.x = ImGui::GetContentRegionAvail().x;
    }

    const float labelMarginLeft = 45.0f;
    const float labelMarginBottom = 22.0f;
    const float marginRight = 15.0f;
    const float marginTop = 15.0f;

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + canvasSize.x, p0.y + canvasSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(p0, p1, IM_COL32(22, 24, 30, 255));
    drawList->AddRect(p0, p1, IM_COL32(55, 60, 75, 255));

    ImVec2 plotMin = ImVec2(p0.x + labelMarginLeft, p0.y + marginTop);
    ImVec2 plotMax = ImVec2(p1.x - marginRight, p1.y - labelMarginBottom);

    if (plotMax.x <= plotMin.x || plotMax.y <= plotMin.y) {
        ImGui::Dummy(canvasSize);
        return;
    }

    drawList->PushClipRect(plotMin, plotMax, true);

    float minX = 0.0f, maxX = 10.0f, minY = -2.0f, maxY = 2.0f;
    computeBounds(minX, maxX, minY, maxY);

    auto toScreen = [&](float x, float y) -> ImVec2 {
        float nx = (x - minX) / (maxX - minX);
        float ny = (y - minY) / (maxY - minY);
        return ImVec2(
            plotMin.x + nx * (plotMax.x - plotMin.x),
            plotMax.y - ny * (plotMax.y - plotMin.y)
        );
    };

    auto toWorld = [&](const ImVec2& scr) -> ImVec2 {
        float nx = (scr.x - plotMin.x) / (plotMax.x - plotMin.x);
        float ny = (plotMax.y - scr.y) / (plotMax.y - plotMin.y);
        return ImVec2(minX + nx * (maxX - minX), minY + ny * (maxY - minY));
    };

    const ImU32 gridColor = IM_COL32(45, 50, 62, 255);
    const ImU32 axisColor = IM_COL32(90, 100, 125, 255);
    const ImU32 textColor = IM_COL32(160, 170, 190, 255);

    const int numYTicks = 5;
    for (int i = 0; i <= numYTicks; i++) {
        float yVal = minY + (maxY - minY) * (static_cast<float>(i) / numYTicks);
        ImVec2 pt = toScreen(minX, yVal);
        drawList->AddLine(ImVec2(plotMin.x, pt.y), ImVec2(plotMax.x, pt.y), gridColor);
    }

    const int numXTicks = 6;
    for (int i = 0; i <= numXTicks; i++) {
        float xVal = minX + (maxX - minX) * (static_cast<float>(i) / numXTicks);
        ImVec2 pt = toScreen(xVal, minY);
        drawList->AddLine(ImVec2(pt.x, plotMin.y), ImVec2(pt.x, plotMax.y), gridColor);
    }

    if (minY <= 0.0f && maxY >= 0.0f) {
        ImVec2 pt0 = toScreen(minX, 0.0f);
        drawList->AddLine(ImVec2(plotMin.x, pt0.y), ImVec2(plotMax.x, pt0.y), axisColor, 1.5f);
    }
    if (minX <= 0.0f && maxX >= 0.0f) {
        ImVec2 pt0 = toScreen(0.0f, minY);
        drawList->AddLine(ImVec2(pt0.x, plotMin.y), ImVec2(pt0.x, plotMax.y), axisColor, 1.5f);
    }

    for (const auto& s : series) {
        if (!s.visible || s.points.size() < 2) continue;

        const size_t nPts = s.points.size();
        size_t stride = (nPts > 1200) ? (nPts / 800) : 1;

        ImVec2 prevScr = toScreen(s.points[0].x, s.points[0].y);
        for (size_t i = stride; i < nPts; i += stride) {
            ImVec2 curScr = toScreen(s.points[i].x, s.points[i].y);
            drawList->AddLine(prevScr, curScr, s.color, s.thickness);
            prevScr = curScr;
        }

        if (isPhasePlot && !s.points.empty()) {
            ImVec2 lastPt = toScreen(s.points.back().x, s.points.back().y);
            drawList->AddCircleFilled(lastPt, 4.0f, s.color);
            drawList->AddCircle(lastPt, 6.0f, IM_COL32(255, 255, 255, 220), 0, 1.5f);
        }
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    bool hovered = (mousePos.x >= plotMin.x && mousePos.x <= plotMax.x &&
                    mousePos.y >= plotMin.y && mousePos.y <= plotMax.y);

    if (hovered && showCrosshairs) {
        drawList->AddLine(ImVec2(plotMin.x, mousePos.y), ImVec2(plotMax.x, mousePos.y), IM_COL32(200, 200, 200, 100));
        drawList->AddLine(ImVec2(mousePos.x, plotMin.y), ImVec2(mousePos.x, plotMax.y), IM_COL32(200, 200, 200, 100));

        ImVec2 wPos = toWorld(mousePos);
        char tip[64];
        if (isPhasePlot) {
            std::snprintf(tip, sizeof(tip), "x: %.3f\nv: %.3f", wPos.x, wPos.y);
        } else {
            std::snprintf(tip, sizeof(tip), "t: %.2f s\ny: %.3f", wPos.x, wPos.y);
        }
        drawList->PopClipRect();
        ImGui::SetTooltip("%s", tip);
        drawList->PushClipRect(plotMin, plotMax, true);
    }

    drawList->PopClipRect();

    char buf[32];
    for (int i = 0; i <= numYTicks; i++) {
        float yVal = minY + (maxY - minY) * (static_cast<float>(i) / numYTicks);
        ImVec2 pt = toScreen(minX, yVal);
        std::snprintf(buf, sizeof(buf), "%.1f", yVal);
        drawList->AddText(ImVec2(p0.x + 4.0f, pt.y - 7.0f), textColor, buf);
    }
    for (int i = 0; i <= numXTicks; i++) {
        float xVal = minX + (maxX - minX) * (static_cast<float>(i) / numXTicks);
        ImVec2 pt = toScreen(xVal, minY);
        std::snprintf(buf, sizeof(buf), "%.1f", xVal);
        drawList->AddText(ImVec2(pt.x - 12.0f, plotMax.y + 4.0f), textColor, buf);
    }

    ImGui::Dummy(canvasSize);

    if (showLegend && !series.empty()) {
        ImGui::Spacing();
        for (size_t i = 0; i < series.size(); i++) {
            auto& s = series[i];
            ImVec4 col = ImColor(s.color);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            std::string cbId = s.name + "##" + title + std::to_string(i);
            ImGui::Checkbox(cbId.c_str(), &s.visible);
            ImGui::PopStyleColor();
            if (i + 1 < series.size()) {
                ImGui::SameLine(0, 14.0f);
            }
        }
    }
}
