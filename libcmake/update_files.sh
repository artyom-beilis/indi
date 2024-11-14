#!/bin/bash
cp -v ../../indi-3rdparty/indi-eqmod/{eqmod,azgti,staradventurergti,staradventurer2i}.cpp .
sed 's/std::unique_ptr<\(.*\)> \(.*\)(\(new .*\));/\1 *\2 = \3;/' -i {eqmod,azgti,staradventurergti,staradventurer2i}.cpp 

cp -v ../../indi-3rdparty/indi-eqmod/indi_eqmod.xml.cmake indi_eqmod.xml
sed 's/@EQMOD_VERSION_MAJOR@.@EQMOD_VERSION_MINOR@/1.3/' -i indi_eqmod.xml


