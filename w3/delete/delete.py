import os

downloadPath = "./w3/default/upload_dest1/"
path = "../upload_dest1/"
files = ""
fileList = sorted(os.listdir(downloadPath))
for file in fileList:
    if os.path.isfile(os.path.join(downloadPath, file)):
        if file[0] != ".":
            files += f"""
                <button class="list-group-item list-group-item-action list-group-item-danger"
                    onclick="
                    fetch('{path}{file}', {{ method: 'DELETE' }})
                        .then(res => {{ if (!res.ok) {{
                            return alert('Error! Response replied with error ' + res.status + '.')
                        }} else {{
                            document.location.reload();
                    }}}});"><div>{file}</div></button>
                """

html = f"""
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <title>Delete a file</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
        integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
</head>

<body>
    <div class="d-flex align-items-center text-center justify-content-center bg-danger-subtle vh-100">
        <div>
            <div class="display-6">Click filename to DELETE</div>
            <div class="list-group"><div>{files}</div></div>
        </div>
    </div>
</body>

</html>
"""

print("Content-Type: text/html\r\n\r\n")
print(html)
