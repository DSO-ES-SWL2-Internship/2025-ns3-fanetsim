# Make symbolic link within ns3 code base
# So that code from the fanet library can be included into build 
# without source file duplication

rm -rf ${PWD}/ns3/src/fanet
ln -s ${PWD}/fanet ${PWD}/ns3/src/fanet
rm ${PWD}/ns3/CMakeUserPresets.json
ln CMakeUserPresets.json ${PWD}/ns3/CMakeUserPresets.json
# cd ns3
# ./ns3 run fanet