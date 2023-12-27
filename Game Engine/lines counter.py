import os

currentDirectory = os.getcwd()

files = os.listdir(currentDirectory)
i = 1
lines = 0
for file in files:
    filename, file_extension = os.path.splitext(file)
    if(file_extension == '.h' or file_extension == '.cpp'):
        numLines = sum(1 for line in open(file))
        print('Lines in ' + os.path.basename(file) + ': ' + str(numLines))
        lines += numLines
       
print('\nTotal lines: ' + str(lines))
input()
