#include <cell/app.hpp>
#include <util/util.hpp>

auto main() -> int {
    try {
        auto state = cell::AppState();
        state.run();
    } catch (std::exception const &exc) {
        eprintln("exception: {}", exc.what());
        return 1;
    }
    return 0;
}
