gcc -Iinclude -c src/main.c
gcc -Iinclude -c src/module.c
gcc -Iinclude -c deps/cglm.c
gcc -Iinclude -c deps/flecs.c
gcc -Iinclude -c deps/flecs_components_cglm.c
gcc -Iinclude -c deps/flecs_components_geometry.c
gcc -Iinclude -c deps/flecs_components_graphics.c
gcc -Iinclude -c deps/flecs_components_gui.c
gcc -Iinclude -c deps/flecs_components_input.c
gcc -Iinclude -c deps/flecs_components_physics.c
gcc -Iinclude -c deps/flecs_components_transform.c
gcc -Iinclude -c deps/flecs_game.c
gcc -Iinclude -c deps/flecs_systems_physics.c
gcc -Iinclude -c deps/flecs_systems_sokol.c
gcc -Iinclude -c deps/flecs_systems_transform.c

gcc \
main.o \
module.o \
cglm.o \
flecs.o \
flecs_components_transform.o \
flecs_components_input.o \
flecs_components_cglm.o \
flecs_components_geometry.o \
flecs_components_graphics.o \
flecs_components_gui.o \
flecs_components_physics.o \
flecs_game.o \
flecs_systems_sokol.o \
flecs_systems_physics.o \
flecs_systems_transform.o \
-lws2_32 -lgdi32