#place here all libraries

#SQLite
IF(USE_SQLITE_DATABASE)
	add_subdirectory(SQLite)
	set(SQLITE_INCLUDE_DIRECTORY ${SQLITE_INCLUDE_DIRECTORY} PARENT_SCOPE)
ENDIF()

