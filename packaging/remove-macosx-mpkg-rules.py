"""
Remove requirements section in Info.plist for Mac OS X Installer package.

Background: There is a bug in the command-line version of PackageMaker
v3.0.4 (179).  When creating a meta-package (bundle of packages), if
any of constituent packages specify installation requirements,
PackageMaker will crash.  The reason is probably that PackageMaker
needs to ask the user (via the GUI, which in this case doesn't exist
from the command line) how to handle each constituents' requirements.

The solution is to bake those requirements into the global
meta-package template (pmdoc) and then remove the requirements from
each constituent package.  This script does the latter by removing a
known key and value from the specified package's Info.plist file.
"""

import sys
from xml.dom.minidom import parse
from optparse import OptionParser

def main():
    parser = OptionParser()
    parser.add_option("-i", "--input", dest="input")
    parser.add_option("-o", "--output", dest="output")
    (options, args) = parser.parse_args()

    if not options.input:
        print "No input given."
        parser.print_help()
        return -1

    try:
        dom = parse(options.input)
    except Exception, err:
        print "Error: ", err
        return -1

    # XML looks something like this:
    # <plist />
    #   <dict>
    #      <key> ... </key>
    #      <string> ... </string>  # this is the value
    #      <key> ... </key>
    #      <string> ... </string>
    #      ...
    #      <key>IFRequirementDicts</key>    # REMOVE THIS
    #      <array>                         # ...AND THIS
    #          <dict>
    #              ...
    #          </dict>
    #      </array>
    #      ...
    #   </dict>
    # </plist>

    plist = dom.getElementsByTagName("key")
    for k in plist:
        if ((k.firstChild.nodeType is k.TEXT_NODE) and
            (k.firstChild.data.strip() == "IFRequirementDicts")):

            sibling = k.nextSibling

            # Skip all whitespace between this key and the upcoming
            # <array>
            while sibling.nodeType is k.TEXT_NODE:
                sibling = sibling.nextSibling

            assert sibling.tagName == "array", "Could not find <array>"

            # Remove nodes
            parent = k.parentNode
            parent.removeChild(k)
            parent.removeChild(sibling)
            k.unlink()
            sibling.unlink()
            break

    xml = dom.toxml()

    if options.output:
        try:
            f = open(options.output, "w")
        except IOError, err:
            print "Error: ", err
            return -1

        f.write(xml)
        f.close()
    else:
        print xml
    
if __name__ == "__main__":
    sys.exit(main())
