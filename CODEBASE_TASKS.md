# Codebase Tasks

* [x] Fix build dependency issue with wayland-scanner on Linux for GLFW. (Completed, not in codebase, environment only)
* [ ] Address `src/nav/NavMeshGenerator.cpp:329` TODO: only remove if there is at least one unconnected edge.
  * *Context:* The `cullTinyFaces` function currently indiscriminately removes polygons with area less than `TINY_POLY` (64). The comment notes this creates holes if all edges are connected.
  * *To implement:* Check if the tiny polygon has any edges that are not shared with any other polygon. If all edges are shared, it shouldn't be removed, as doing so will create a hole in the mesh.
