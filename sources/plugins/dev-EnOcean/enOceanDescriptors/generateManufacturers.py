#!/usr/local/bin/python
# coding: utf-8
#-------------------------------------------------------------------------------
# CPP code generation script for EnOcean manufacturers

import sys
import os.path
import xml.etree.ElementTree
import string

import cppClass
import cppHelper
import xmlHelper
import util


#-------------------------------------------------------------------------------
# Input parameters
xmlInputFilePath = sys.argv[1]
headerPath = sys.argv[2]
sourcePath = sys.argv[3]




#-------------------------------------------------------------------------------


xmlRootNode = xml.etree.ElementTree.parse(xmlInputFilePath).getroot()
if xmlRootNode.tag != "manufacturers":
   raise Exception("getAllNodes : Invalid root \"" + xmlRootNode.tag + "\", \"manufacturers\" expected")

manufacturersClassName = "CManufacturers"
manufacturersClass = cppClass.CppClass(manufacturersClassName)
manufacturersClass.addSubType(cppClass.CppEnumType("EManufacturerIds", \
   xmlHelper.getEnumValues(inNode=xmlRootNode, foreachSubNode="manufacturer", enumValueNameTag="name", enumValueTag="id"), cppClass.PUBLIC))
manufacturersClass.addMember(cppClass.CppMember("ManufacturersMap", "std::map<unsigned int, std::string>", cppClass.PRIVATE, cppClass.STATIC | cppClass.CONST, \
   cppHelper.getMapInitCode(xmlHelper.getEnumValues(inNode=xmlRootNode, foreachSubNode="manufacturer", enumValueNameTag="name"))))
manufacturersClass.addMethod(cppClass.CppMethod("toManufacturerId", manufacturersClassName + "::EManufacturerIds", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
   "   if (ManufacturersMap.find(id) == ManufacturersMap.end())\n" \
   "      throw std::out_of_range(\"Unknown manufacturer\");\n" \
   "   return static_cast<EManufacturerIds>(id);\n"))
manufacturersClass.addMethod(cppClass.CppMethod("name", "const std::string&", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
   "   try {\n" \
   "      return ManufacturersMap.at(id);\n" \
   "   } catch(std::out_of_range&) {\n" \
   "      static const std::string UnknownManufacturer(\"Unknown manufacturer\");\n" \
   "      return UnknownManufacturer;\n" \
   "   }"))


# Generate Header
util.createParentDir(headerPath)
with open(headerPath, 'w') as cppHeaderFile:

   cppHeaderFile.write("// Generated file, don't modify\n")
   cppHeaderFile.write("#pragma once\n")
   cppHeaderFile.write("\n")

   manufacturersClass.generateHeader(cppHeaderFile)

# Generate Source
util.createParentDir(sourcePath)
with open(sourcePath, 'w') as cppSourceFile:

   cppSourceFile.write("// Generated file, don't modify\n")
   cppSourceFile.write("#include \"stdafx.h\"\n")
   cppSourceFile.write("#include \"{}\"\n".format(os.path.basename(headerPath)))
   cppSourceFile.write("\n")

   manufacturersClass.generateSource(cppSourceFile)

sys.exit(util.getError())
