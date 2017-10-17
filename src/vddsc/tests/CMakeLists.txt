include(Criterion)

idlc_generate(RoundTrip RoundTrip.idl)
idlc_generate(Space Space.idl)
idlc_generate(TypesArrayKey TypesArrayKey.idl)
add_criterion_executable(criterion_vddsc .)
target_link_libraries(criterion_vddsc RoundTrip Space TypesArrayKey vddsc OSAPI)

# Setup environment for config-tests
set(Criterion_vddsc_config_simple_udp_file "${CMAKE_CURRENT_LIST_DIR}/config_simple_udp.xml")
set(Criterion_vddsc_config_simple_udp_uri "file://${Criterion_vddsc_config_simple_udp_file}")
set(Criterion_vddsc_config_simple_udp_max_participants "0")
set_tests_properties(
	Criterion_vddsc_config_simple_udp
	PROPERTIES
		REQUIRED_FILES ${Criterion_vddsc_config_simple_udp_file}
		ENVIRONMENT "VORTEXDDS_URI=${Criterion_vddsc_config_simple_udp_uri};MAX_PARTICIPANTS=${Criterion_vddsc_config_simple_udp_max_participants}"

)
configure_file("config_env.h.in" "config_env.h")
