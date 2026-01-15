vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO cameron314/concurrentqueue
    REF v1.0.4
    SHA512 a27306d1a7ad725daf5155a8e33a93efd29839708b2147ba703d036c4a92e04cbd8a505d804d2596ccb4dd797e88aca030b1cb34a4eaf09c45abb0ab55e604ea
)

file(INSTALL "${SOURCE_PATH}/concurrentqueue.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/moodycamel")
file(INSTALL "${SOURCE_PATH}/blockingconcurrentqueue.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/moodycamel")
file(INSTALL "${SOURCE_PATH}/lightweightsemaphore.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/moodycamel")
file(INSTALL "${SOURCE_PATH}/LICENSE.md" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER disabled)
