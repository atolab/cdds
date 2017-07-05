include(Criterion)

idlc_generate(RoundTrip RoundTrip.idl)
add_criterion_executable(criterion_vddsc .)
target_link_libraries(criterion_vddsc RoundTrip vddsc OSAPI)

