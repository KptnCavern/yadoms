#-------------------------------------------------------------------------------
# CPP code generation script for EnOcean EEP profiles

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
classes = []


xmlRootNode = xml.etree.ElementTree.parse(xmlInputFilePath).getroot()
if xmlRootNode.tag != "eep":
   raise Exception("getAllNodes : Invalid root \"" + xmlRootNode.tag + "\", \"eep\" expected")
xmlProfileNode = xmlRootNode.find("profile")


# CRorgs : Main Rorgs class, listing Rorg messages
rorgsClass = cppClass.CppClass("CRorgs")
classes.append(rorgsClass)
rorgsClass.addSubType(cppClass.CppEnumType("ERorgIds", \
   xmlHelper.getEnumValues(inNode=xmlProfileNode, foreachSubNode="rorg", enumValueNameTag="title", enumValueTag="number"), cppClass.PUBLIC))
rorgsClass.addMember(cppClass.CppMember("RorgMap", "std::map<unsigned int, std::string>", cppClass.PRIVATE, cppClass.STATIC | cppClass.CONST, \
   cppHelper.getMapInitCode(xmlHelper.getEnumValues(inNode=xmlProfileNode, foreachSubNode="rorg", enumValueNameTag="title"))))
rorgsClass.addMethod(cppClass.CppMethod("toRorgId", "CRorgs::ERorgIds", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
   "   if (RorgMap.find(id) == RorgMap.end())\n" \
   "      throw std::out_of_range(\"Unknown rorg\");\n" \
   "   return static_cast<ERorgIds>(id);\n"))
rorgsClass.addMethod(cppClass.CppMethod("name", "const std::string&", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
   "   try {\n" \
   "      return RorgMap.at(id);\n" \
   "   } catch(std::out_of_range&) {\n" \
   "      static const std::string UnknownRorg(\"Unknown rorg\");\n" \
   "      return UnknownRorg;\n" \
   "   }"))


# IRorg : Rorg interface
irorgClass = cppClass.CppClass("IRorg")
classes.append(irorgClass)
irorgClass.addMethod(cppClass.CppMethod("id", "unsigned int", "", cppClass.PUBLIC, cppClass.CONST | cppClass.PURE_VIRTUAL))
irorgClass.addMethod(cppClass.CppMethod("title", "const std::string&", "", cppClass.PUBLIC, cppClass.CONST | cppClass.PURE_VIRTUAL))
irorgClass.addMethod(cppClass.CppMethod("fullname", "const std::string&", "", cppClass.PUBLIC, cppClass.CONST | cppClass.PURE_VIRTUAL))
irorgClass.addMethod(cppClass.CppMethod("dump", "std::string", "", cppClass.PUBLIC, cppClass.CONST | cppClass.PURE_VIRTUAL))




for xmlRorgNode in xmlProfileNode.findall("rorg"):

   # Rorg telegram classes
   rorgClassName = "C" + xmlRorgNode.find("telegram").text + "Telegram"
   rorgClass = cppClass.CppClass(rorgClassName, createDefaultCtor=False)
   rorgClass.inheritFrom("IRorg", cppClass.PUBLIC)
   classes.append(rorgClass)
   rorgClass.addMember(cppClass.CppMember("m_erp1Data", "std::vector<unsigned char>&", cppClass.PRIVATE, cppClass.CONST))
   rorgClass.addConstructor(cppClass.CppClassConstructor(args="const std::vector<unsigned char>& erp1Data", init="m_erp1Data(erp1Data)"))
   rorgClass.addMethod(cppClass.CppMethod("id", "unsigned int", "", cppClass.PUBLIC, cppClass.OVERRIDE | cppClass.CONST, "   return " + xmlRorgNode.find("number").text + ";"))
   rorgClass.addMethod(cppClass.CppMethod("title", "const std::string&", "", cppClass.PUBLIC, cppClass.OVERRIDE | cppClass.CONST, \
   "   static const std::string title(\"" + xmlRorgNode.find("title").text + "\");\n" \
   "   return title;"))
   rorgClass.addMethod(cppClass.CppMethod("fullname", "const std::string&", "", cppClass.PUBLIC, cppClass.OVERRIDE | cppClass.CONST, \
   "   static const std::string fullname(\"" + xmlRorgNode.find("fullname").text + "\");\n" \
   "   return fullname;"))
   rorgClass.addMethod(cppClass.CppMethod("dump", "std::string", "", cppClass.PUBLIC, cppClass.OVERRIDE | cppClass.CONST, \
   "   std::stringstream ss;\n" \
   "   ss << std::setfill('0') << std::setw(2) << std::hex;\n" \
   "   for (auto it = m_erp1Data.begin(); it != m_erp1Data.end(); ++it)\n" \
   "      ss << *it << \" \";\n" \
   "   return ss.str();"))

   rorgClass.addSubType(cppClass.CppEnumType("EFuncIds", xmlHelper.getEnumValues(inNode=xmlRorgNode, foreachSubNode="func", enumValueNameTag="title", enumValueTag="number"), cppClass.PUBLIC))
   rorgClass.addMember(cppClass.CppMember("FuncMap", "std::map<unsigned int, std::string>", cppClass.PRIVATE, cppClass.STATIC | cppClass.CONST, \
      cppHelper.getMapInitCode(xmlHelper.getEnumValues(inNode=xmlRorgNode, foreachSubNode="func", enumValueNameTag="title"))))
   rorgClass.addMethod(cppClass.CppMethod("toFuncId", rorgClassName + "::EFuncIds", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
      "   if (FuncMap.find(id) == FuncMap.end())\n" \
      "      throw std::out_of_range(\"Unknown func\");\n" \
      "   return static_cast<EFuncIds>(id);\n"))
   rorgClass.addMethod(cppClass.CppMethod("toFuncName", "const std::string&", "unsigned int id", cppClass.PUBLIC, cppClass.STATIC, \
      "   try {\n" \
      "      return FuncMap.at(id);\n" \
      "   } catch(std::out_of_range&) {\n" \
      "      static const std::string UnknownFunc(\"Unknown func\");\n" \
      "      return UnknownFunc;\n" \
      "   }"))


   # Func classes
   for xmlFuncNode in xmlRorgNode.findall("func"):
      funcClass = cppClass.CppClass("C" + xmlRorgNode.find("telegram").text + "_" + cppHelper.toCppName(xmlFuncNode.find("title").text))
      classes.append(funcClass)
      funcClass.addMethod(cppClass.CppMethod("id", "unsigned int", "", cppClass.PUBLIC, cppClass.STATIC, "   return " + xmlFuncNode.find("number").text + ";"))
      funcClass.addMethod(cppClass.CppMethod("title", "const std::string&", "", cppClass.PUBLIC, cppClass.STATIC, \
      "   static const std::string title(\"" + xmlFuncNode.find("title").text + "\");\n" \
      "   return title;"))




# Generate Header
util.createParentDir(headerPath)
with open(headerPath, 'w') as cppHeaderFile:

   cppHeaderFile.write("// Generated file, don't modify\n")
   cppHeaderFile.write("#pragma once\n")
   cppHeaderFile.write("\n")

   for oneClass in classes:
      oneClass.generateHeader(cppHeaderFile)

# Generate Source
util.createParentDir(sourcePath)
with open(sourcePath, 'w') as cppSourceFile:

   cppSourceFile.write("// Generated file, don't modify\n")
   cppSourceFile.write("#include \"stdafx.h\"\n")
   cppSourceFile.write("#include \"{}\"\n".format(os.path.basename(headerPath)))
   cppSourceFile.write("\n")

   for oneClass in classes:
      oneClass.generateSource(cppSourceFile)
