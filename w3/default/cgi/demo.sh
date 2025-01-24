web_color=('AliceBlue' 'AntiqueWhite' 'Aqua' 'Aquamarine' 'Azure' 'Beige' 'Bisque' 'Black' 'BlanchedAlmond' 'Blue' 'BlueViolet' 'Brown' 'Burlywood' 'CadetBlue' 'Chartreuse' 'Chocolate' 'Coral' 'CornflowerBlue' 'Cornsilk' 'Crimson' 'Cyan' 'DarkBlue' 'DarkCyan' 'DarkGoldenrod' 'DarkGray' 'DarkGreen' 'DarkKhaki' 'DarkMagenta' 'DarkOliveGreen' 'DarkOrange' 'DarkOrchid' 'DarkRed' 'DarkSalmon' 'DarkSeaGreen' 'DarkSlateBlue' 'DarkSlateGray' 'DarkTurquoise' 'DarkViolet' 'DeepPink' 'DeepSkyBlue' 'DimGray' 'DodgerBlue' 'Firebrick' 'FloralWhite' 'ForestGreen' 'Fuchsia' 'Gainsboro' 'GhostWhite' 'Gold' 'Goldenrod' 'Gray' 'Green' 'GreenYellow' 'Honeydew' 'HotPink' 'IndianRed' 'Indigo' 'Ivory' 'Khaki' 'Lavender' 'LavenderBlush' 'LawnGreen' 'LemonChiffon' 'LightBlue' 'LightCoral' 'LightCyan' 'LightGoldenrodYellow' 'LightGray' 'LightGreen' 'LightPink' 'LightSalmon' 'LightSeaGreen' 'LightSkyBlue' 'LightSlateGray' 'LightSteelBlue' 'LightYellow' 'Lime' 'LimeGreen' 'Linen' 'Magenta' 'Maroon' 'MediumAquamarine' 'MediumBlue' 'MediumOrchid' 'MediumPurple' 'MediumSeaGreen' 'MediumSlateBlue' 'MediumSpringGreen' 'MediumTurquoise' 'MediumVioletRed' 'MidnightBlue' 'MintCream' 'MistyRose' 'Moccasin' 'NavajoWhite' 'Navy' 'OldLace' 'Olive' 'OliveDrab' 'Orange' 'OrangeRed' 'Orchid' 'PaleGoldenrod' 'PaleGreen' 'PaleTurquoise' 'PaleVioletRed' 'PapayaWhip' 'PeachPuff' 'Peru' 'Pink' 'Plum' 'PowderBlue' 'Purple' 'Red' 'RosyBrown' 'RoyalBlue' 'SaddleBrown' 'Salmon' 'SandyBrown' 'SeaGreen' 'Seashell' 'Sienna' 'Silver' 'SkyBlue' 'SlateBlue' 'SlateGray' 'Snow' 'SpringGreen' 'SteelBlue' 'Tan' 'Teal' 'Thistle' 'Tomato' 'Turquoise' 'Violet' 'Wheat' 'White' 'WhiteSmoke' 'Yellow' 'YellowGreen')
read -r input
color=$(echo "$input" | sed -n 's/.*color=\([^&]*\).*/\1/p')
color=$(echo "$color" | sed 's/+/ /g;s/%/\\x/g')

if [ $color -lt 1 ] || [ $color -gt 140 ]; then
  	message="Sorry, buddy, there ARE limits, y'know."
	color=
else
	((color--))
	color=${web_color[$color]}
fi

echo "Content-Type: text/html; charset=utf-8"
echo -e "\r\n\r\n"
echo ""

cat << EOF
<!DOCTYPE html>
<html lang="en">
	<head>
		<meta charset="UTF-8">
		<title>Web Color</title>
		<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet"
			integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
	</head>
	<body style="background-color: $color">
		<div class="d-flex align-items-center text-center justify-content-center vh-100">
			<div class="display-4">$color</div>
			<div class="display-6">$message</div>
		</div>
	</body>
</html>
EOF
