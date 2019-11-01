#include <vector>

namespace ImGui
{
void Plot(const std::string &label,
          const std::vector<double> &ys,
          const std::vector<double> &xs,
          ImVec2 size);
}