#include <cell/app.hpp>
#include <util/util.hpp>

auto main() -> int {
    using namespace cell;

    try {
        AppState state = AppState();
        state.run();
    } catch (std::exception const &exc) {
        eprintln("exception: {}", exc.what());
        return 1;
    }

    return 0;
}
