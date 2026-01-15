1. Header-Only, Just Vendor It
moodycamel is header-only (just concurrentqueue.h and optionally blockingconcurrentqueue.h). You could:
deps/moodycamel/concurrentqueue.h
Minimal maintenance burden since it rarely changes. Not ideal if you're strict about vcpkg-for-everything, but pragmatic.
2. Create a Local vcpkg Port
In your repo:
vcpkg-ports/
  moodycamel-concurrentqueue/
    portfile.cmake
    vcpkg.json
json// vcpkg.json
{
  "name": "moodycamel-concurrentqueue",
  "version": "1.0.4",
  "description": "Lock-free concurrent queue",
  "homepage": "https://github.com/cameron314/concurrentqueue"
}
cmake# portfile.cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO cameron314/concurrentqueue
    REF v1.0.4
    SHA512 ...
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
file(INSTALL "${SOURCE_PATH}/LICENSE.md" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
Then use --overlay-ports=vcpkg-ports.
3. Stick with eventpp
If the vcpkg friction isn't worth it and eventpp is "good enough," the performance gain may not justify the hassle. Profile first if you haven't—the bottleneck might be elsewhere entirely.

What's your preference? If you're leaning toward moodycamel, vendoring the header is the lowest-friction path. It's a single file with a permissive license.
