add_executable( SRSTwistS SRS/TwistS.cpp )
add_test(        SRSTwistS SRSTwistS    )

add_executable( SRSTwistL SRS/TwistL.cpp )
add_test(        SRSTwistL SRSTwistL    )

add_executable( SRSTSpin SRS/TSpin.cpp )
add_test(        SRSTSpin SRSTSpin    )

target_link_libraries(SRSTwistS LINK_PUBLIC Tetris)
target_link_libraries(SRSTwistL LINK_PUBLIC Tetris)
target_link_libraries(SRSTSpin LINK_PUBLIC Tetris)
