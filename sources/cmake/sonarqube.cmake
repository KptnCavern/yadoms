

macro(create_sonar_project_properties_file projectName projectVersion cppCheckReport outputFile)

message(STATUS "  Create SonarQube configuration file ${outputFile} for projectName ${projectName}(v${projectVersion})")
		
		get_sonarqube_defines(SONARQUBE_DEFINES ${projectName})
		get_sonarqube_sources(SONARQUBE_SOURCES ${projectName})
		get_sonarqube_includes(SONARQUBE_INCLUDES ${projectName})
		get_sonarqube_tests(SONARQUBE_TESTS ${projectName})
		
		list_to_string(CONTENT
			sonar.projectBaseDir=${CMAKE_SOURCE_DIR}
			sonar.projectKey=${projectName}
			sonar.projectVersion=${projectVersion}
			sonar.projectName=${projectName}
			sonar.language=c++
			sonar.sources=${SONARQUBE_SOURCES}
			sonar.cxx.coverage.forceZeroCoverage=False
			sonar.cxx.includeDirectories=${SONARQUBE_INCLUDES}
         
         sonar.cxx.cppcheck.path=${CPPCHECK_EXECUTABLE}        # TODO utile ?
         sonar.cxx.cppcheck.reportPath=${cppCheckReport}
         
			${SONARQUBE_TESTS}
		)
		set(CONTENT "${CONTENT}\nsonar.cxx.defines=${SONARQUBE_DEFINES}")
	
      
		FILE(WRITE ${outputFile} ${CONTENT})
		
		# get_msbuild_entry(SONAR_LANGUAGE sonar.language c++)
		# get_msbuild_entry(SONAR_SOURCES sonar.inclusions  ${SONARQUBE_SOURCES})
		# get_msbuild_entry(SONAR_FORCE_ZERO_COVERAGE sonar.cxx.coverage.forceZeroCoverage False)
		# get_msbuild_entry(SONAR_INCLUDE_DIRECTORIES sonar.cxx.includeDirectories ${SONARQUBE_INCLUDES})
		# get_msbuild_entry(SONAR_HOST sonar.host.url http://localhost:9000)
		# get_root(LOCAL_REPO_PATH)
		
		
		# get_target_property(testTagets ${projectName} TESTED_BY)
		# set(unitReportPaths)
		# SET(i 1)
		# foreach (testTarget ${testTargets})
			# set(unitReportPaths ${unitReportPaths} ${LOCAL_REPO_PATH}/TestResultsSQ/File${i}.trx)
			# MATH(EXPR i "${i}+1")
		# ENDFOREACH()
		# JOIN("${unitReportPaths}" "," unitReportPaths)
		
		# if (unitReportPaths)
		# get_msbuild_entry(SONAR_TESTS sonar.cxx.vstest.reportsPaths ${unitReportPaths})
		# endif()
		# # get_target_property(externalSources ${projectName} EXTERNAL_SOURCES)
		# # get_root(LOCAL_REPO_PATH)
		# # STRING(REGEX REPLACE "${LOCAL_REPO_PATH}/" "" externalSources "${externalSources}" )
		# # STRING(REGEX REPLACE ";" "," externalSources "${externalSources}" )
		# # get_msbuild_entry(SONAR_HOST sonar.host.url http://localhost:9000)
		
		# #get_msbuild_entry(COVERAGE_UNIT_TESTS sonar.cxx.coverage.reportPath UnitTestCoverageResults.xml)
		# #get_msbuild_entry(COVERAGE_ACCEPTANCE_TESTS sonar.cxx.coverage.itReportPath AcceptanceTestCoverageResults.xml)
		# get_msbuild_entry(COVERAGE_ALL_TESTS sonar.cxx.coverage.overallReportPath ${LOCAL_REPO_PATH}/AllTestCoverageResults.xml)
		# get_msbuild_entry(COVERAGE_ACCEPTANCE_TESTS sonar.cxx.coverage.itReportPath ${LOCAL_REPO_PATH}/AllTestCoverageResults.xml)
		# get_msbuild_entry(CPPCHECK_REPORTS sonar.cxx.cppcheck.reportPath *_cppcheck.xml)
	
endmacro()



macro(get_msbuild_entry output key value)

	set(${output} "<Property Name=\"${key}\">${value}</Property>")

endmacro()

MACRO(get_sonarqube_tests output projectName)

	get_target_property(testTagets ${projectName} TESTED_BY)
	set(SONARQUBE_JUNIT_RESULTS)
	foreach (testTarget ${testTargets})
		get_target_property(kind ${testTarget} TEST_FRAMEWORK)
		get_target_property(reportPath ${testTarget} TEST_REPORT)
		if ("${kind}" STREQUAL "GTest")
			set(SONARQUBE_JUNIT_RESULTS ${SONARQUBE_JUNIT_RESULTS} ${reportPath})
		elseif ("${kind}" STREQUAL "NUnit")
			get_target_property(reportPath ${testTarget} TEST_REPORT_JUNIT)
			set(SONARQUBE_JUNIT_RESULTS ${SONARQUBE_JUNIT_RESULTS} ${reportPath})
		else()
			message("Impossible de d�terminer le type de test pour ${testProject}")
		endif()
	endforeach()
	
	if (SONARQUBE_JUNIT_RESULTS)
		JOIN("${SONARQUBE_JUNIT_RESULTS}" "," SONARQUBE_JUNIT_RESULTS)
		set(${output} sonar.cxx.xunit.reportPath=${SONARQUBE_JUNIT_RESULTS})
	endif()
	

ENDMACRO()

MACRO(get_sonarqube_sources output projectName)

	get_target_property(sources ${projectName} SOURCES)
   if(MSVC)
      # Remove .rc files
      LIST(FILTER sources EXCLUDE REGEX "^.*\.rc$")
   endif()
   
	STRING(REGEX REPLACE ";" "," sources "${sources}" )
	SET(${output} ${sources})
	
ENDMACRO()

MACRO(get_sonarqube_includes output projectName)

message ("CMAKE_INCLUDE_PATH=${CMAKE_INCLUDE_PATH}")

	# TODO voir si utile get_std_headers_path(stdHeaders)
	get_target_property(includeDirectories ${projectName} INCLUDE_DIRECTORIES)
	set(includeDirectories ${includeDirectories}
		"C:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt"#TODO ne pas laisser en dur
		"C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/include"#TODO ne pas laisser en dur
		${stdHeaders}
	)
	STRING(REGEX REPLACE ";" "," includeDirectories "${includeDirectories}" )
	SET(${output} ${includeDirectories})

ENDMACRO()

MACRO(get_sonarqube_defines output projectName)

	get_directory_property( COMPILE_DEFS DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
	get_target_property(targetDefines ${projectName} COMPILE_DEFINITIONS)
	IF (targetDefines)
		set(COMPILE_DEFS ${COMPILE_DEFS} ${targetDefines})
	ENDIF()
	
	
	STRING(REGEX REPLACE "=" " " COMPILE_DEFS "${COMPILE_DEFS}" )
	STRING(REPLACE ";" "\\\\n\\\\\\\n" COMPILE_DEFS "${COMPILE_DEFS}" )
	
	SET(${output} ${COMPILE_DEFS})
	
ENDMACRO()

MACRO(get_sonarqube_coverage output projectName)

	get_target_property(testTagets ${projectName} TESTED_BY)
	set(SONARQUBE_UNIT)
	set(SONARQUBE_ACCEPTANCE)
	foreach (testTarget ${testTargets})
		get_target_property(kind ${testTarget} TEST_KIND)
		get_target_property(reportPath ${testTarget} COVERAGE_REPORT_XML)
		if ("${kind}" STREQUAL "UNIT")
			set(SONARQUBE_UNIT ${SONARQUBE_UNIT} ${reportPath})
		elseif ("${kind}" STREQUAL "ACCEPTANCE")
			set(SONARQUBE_ACCEPTANCE ${SONARQUBE_ACCEPTANCE} ${reportPath})
		else()
			message("Impossible de d�terminer le type de test pour ${testProject} (ACCEPTANCE ou UNIT)")
		endif()
	endforeach()
	
	SET(COVERAGE_UNIT_STEPS)
	SET(COVERAGE_ACCEPTANCE_STEPS)
	SET(COVERAGE_ALL_STEPS)
	if (SONARQUBE_UNIT)
		JOIN("${SONARQUBE_UNIT}" "," SONARQUBE_UNIT)
		set(COVERAGE_UNIT_STEPS sonar.cxx.coverage.reportPath=${SONARQUBE_UNIT})
	endif()
	if (SONARQUBE_ACCEPTANCE)
		JOIN("${SONARQUBE_ACCEPTANCE}" "," SONARQUBE_ACCEPTANCE)
		set(COVERAGE_UNIT_STEPS sonar.cxx.coverage.itReportPath=${SONARQUBE_ACCEPTANCE})
	endif()
	IF (SONARQUBE_UNIT OR SONARQUBE_ACCEPTANCE)
		get_target_property(report ${projectName} COVERAGE_REPORT_XML_ALL)
		SET(COVERAGE_ALL_STEPS sonar.cxx.coverage.overallReportPath=${report})
	ENDIF()
	
	SET(${output} ${COVERAGE_UNIT_STEPS} ${COVERAGE_ACCEPTANCE_STEPS} ${COVERAGE_ALL_STEPS})
	
ENDMACRO()


MACRO(get_root output)
   set(${output} "D:/Projets/Domotique/yadoms") #TODO en dur pour test
ENDMACRO()


# Converts a CMake list to a string containing elements separated by a separator
MACRO(list_to_string output)
   set(${output})
   set(separator)
   FOREACH(line ${ARGN})
      set(${output} "${${output}}${separator}${line}")
      set(separator "\n")
   ENDFOREACH(line)
ENDMACRO()


function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()
