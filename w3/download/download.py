import os

downloadPath = "./w3/default/upload_dest1/"
path = "../upload_dest1/"
files = ""
fileList = sorted(os.listdir(downloadPath))
for file in fileList:
    if os.path.isfile(os.path.join(downloadPath, file)):
        if file[0] != ".":
            files += f"""
                <a class="list-group-item list-group-item-action list-group-item-success"
                    href={path}{file} download><div>{file}</div></a>
                """

html = f"""
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <title>Download a file</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
        integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>

<body>
    <div class="d-flex align-items-center text-center justify-content-center bg-success-subtle vh-100">
        <div>
            <div class="display-6">Click filename to download</div>
            <div class="list-group"><div>{files}</div></div>
        </div>
    </div>
</body>

</html>
"""

print("Content-Type: text/html\r\n\r\n")
print(html)
