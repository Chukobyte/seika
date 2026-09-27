# gamecontrollerdb.txt

Vendored copy of the SDL Game Controller DB, used as input to `seika_gamepad_db_gen`
(`seika/tools/gen_gamepad_db.c`) to produce `seika/seika/input/gamepad_db_generated.c`.

Source: https://github.com/mdqinc/SDL_GameControllerDB
License: zlib (see `LICENSE` in this directory)

To refresh: re-download `gamecontrollerdb.txt` from the source above, then re-run the
generator and commit the regenerated `gamepad_db_generated.c`.
