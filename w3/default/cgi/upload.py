import cgi
import os
import cgitb
from pathlib import Path

cgitb.enable()

form = cgi.FieldStorage()

fileToWrite = form['file']
uploadPathInput = form.getvalue("uploadPath")
uploadPath = Path(os.path.join(uploadPathInput))

uploadDirectory = "./www/main/cgi-bin/uploads/"
if uploadPathInput and uploadPath.exists():
    uploadDirectory = uploadPath

print("Content-Type: text/html; charset=utf-8")
print()

if fileToWrite.filename:
    filename = os.path.basename(fileToWrite.filename)
    filepath = os.path.join(uploadDirectory, filename)

    try:
        with open(filepath, 'wb') as output:
            while True:
                chunk = fileToWrite.file.read(1024)
                if not chunk:
                    break
                output.write(chunk)

        message = f"<i>'{filename}'</i><br>has been <b>successfully uploaded</b> to<br>'{
            uploadDirectory}'"
    except Exception as e:
        message = f"Error encountered during saving to file: {e}"
else:
    message = "No file selected for upload"

html = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Upload</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
        integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>
<body>
    <div class="d-flex align-items-center text-center justify-content-center bg-secondary-subtle vh-100">
        <div class="display-6"> {message} </div class>
    </div>
</body>
</html>
"""

print(html)
