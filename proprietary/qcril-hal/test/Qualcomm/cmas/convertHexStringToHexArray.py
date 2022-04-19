import argparse
import sys

myDescription = '''
Script to convert a hex string to hex array.
'''

def parseCmdLines():
    parser = argparse.ArgumentParser(
        description= myDescription, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-s", "--string",
                        required=True,
                        metavar="<hex-strin>",
                        help="""Hex string like 012AFF0910""")
    return parser.parse_args()

def main():
    args = parseCmdLines()
    my_string = args.string

    my_hex = my_string.decode('hex')
    print ("size = {}".format(len(my_string)/2))
    output = ", ".join(hex(ord(n)) for n in my_hex)
    print(output)

    return 0

if __name__ == '__main__':
    sys.exit(main())
