# TODO depending on requirements we can add/remove options as needed,
# these are examples of generators we'll need as a minimum.
# Perhaps we should also consider options for building subset of all docs.
# When a certain doc is related to a target, no option is needed; you can simply check if the target exists
# (i.e. if a target 'vddsc' exists, build vddsc api docs). And possibly make the target definition dependent on an option.
option(BUILD_DOCS_HTML, "Generate HTML documentation" ON)
option(BUILD_DOCS_PDF, "Generate PDF documentation" ON)
set(BUILD_DOCS_HTML ON CACHE BOOL "Generate HTML documentation")
set(BUILD_DOCS_PDF ON CACHE BOOL "Generate PDF documentation")

find_program(SPHINX_EXECUTABLE NAMES sphinx-build DOC "Sphinx documentation builder")
if (NOT SPHINX_EXECUTABLE)
  message(FATAL_ERROR "Unable to find sphinx-build executable")
endif()

find_package(Doxygen)
if (NOT Doxygen_FOUND)
  message(FATAL_ERROR "Unable to find Doxygen")
endif()

# Generate vddsc API docs in XML format using Doxygen
# The XML will serve as input for sphinx' breathe plugin
if (TARGET ${CMAKE_PROJECT_NAME}::vddsc)
  # Process doxygen configuration file, for vddsc
  set(doxy_conf_project "${CMAKE_PROJECT_NAME} C API Documentation")
  set(doxy_conf_outputdir "vddsc_api")
  set(doxy_conf_input "${CMAKE_SOURCE_DIR}/include/dds.h ${CMAKE_SOURCE_DIR}/include/dds")
  configure_file(Doxyfile.in Doxyfile @ONLY)

  add_custom_target(vddsc_docs
    ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Doxygen for API docs generation"
    VERBATIM
  )

  # Remove generated files when cleaning the build tree
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${doxy_conf_outputdir})

  # Add vddsc api docs to sphinx' breathe projects
  set(sph_conf_breathe_projs "\"vddsc_api\": \"${doxy_conf_outputdir}/xml\"")
endif()

# Process sphinx configuration file
set(sph_conf_author "PrismTech")
string(TIMESTAMP sph_conf_copyright "%Y, PrismTech")
set(sph_conf_version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(sph_conf_release "${PROJECT_VERSION}")
configure_file(conf.py.in conf.py @ONLY)

# Define a list of output formats (-b option for sphinx-build)
set(docs_builders "")
if (BUILD_DOCS_HTML)
  list(APPEND docs_builders html)
endif()
if (BUILD_DOCS_PDF)
  list(APPEND docs_builders latex)
  # Creating pdf from latex requires latexmk (which depends on perl, latexpdf et. al)
  find_program(LATEXMK_EXECUTABLE NAMES latexmk DOC "LateX PDF Generator")
  if (NOT LATEXMK_EXECUTABLE)
    message(FATAL_ERROR, "Unable to find latexmk executable")
  endif()
endif()

# Define custom commands for running sphinx-build for different docs builders
set(docs_outputs "")
foreach(builder ${docs_builders})
  set(docs_builder_output "docs_${builder}_output")
  # Log stdout (not stderr) to a file instead of messing up build output
  set(docs_builder_log "sphinx-build-${builder}.log")

  add_custom_command(
    OUTPUT ${docs_builder_output}
    COMMAND ${SPHINX_EXECUTABLE}
      -b ${builder}
      -d ${CMAKE_CURRENT_BINARY_DIR}/cache
      -c ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_SOURCE_DIR}/docs
      ${CMAKE_CURRENT_BINARY_DIR}/${builder}
      > ${docs_builder_log}
    COMMENT "Running Sphinx for ${builder} output"
    VERBATIM
  )

  # Create a pdf from the latex builder output, by appending a latexmk command
  # TODO look into rinohtype as an alternative (don't bother with rst2pdf, it's no good)
  if(builder STREQUAL latex)
    add_custom_command(
      OUTPUT ${docs_builder_output}
      COMMAND ${LATEXMK_EXECUTABLE}
        -interaction=nonstopmode
        -silent
        -output-directory=${builder}
        -pdf -dvi- -ps- -cd ${builder}/${CMAKE_PROJECT_NAME}.tex
      APPEND
      VERBATIM
    )
  endif()

  # OUTPUT is a fake target / symbolic name, not an actual file
  set_property(SOURCE ${docs_builder_output} PROPERTY SYMBOLIC 1)
  # Remove generated files when cleaning the build tree
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${builder} ${docs_builder_log})
  # Include this builder as a dependency of the general 'docs' target
  list(APPEND docs_outputs ${docs_builder_output})
endforeach()

add_custom_target(docs ALL DEPENDS vddsc_docs ${docs_outputs})

if (BUILD_DOCS_HTML)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION "docs")
endif()
if (BUILD_DOCS_PDF)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latex/${CMAKE_PROJECT_NAME}.pdf DESTINATION "docs")
endif()
