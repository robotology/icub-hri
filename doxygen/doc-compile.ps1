
$SOURCE="../"
$DESTINATION="generated-from-xml"

if (test-path doc) {
    rm doc -Recurse
}

if (test-path $DESTINATION) {
    rm $DESTINATION -Recurse
}

if (!(test-path $DESTINATION)) {
    mkdir $DESTINATION
}

ls -Path $SOURCE -Filter *.xml -Recurse | `
foreach-object {xsltproc --output $DESTINATION/$_.dox `
$env:YARP_ROOT\scripts\yarp-module.xsl $_.fullname}

doxygen ./generate.txt
