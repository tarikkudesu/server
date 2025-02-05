<head>
	<style>
		.submit-btn {
			background-color: rgb(35, 40, 47);
			border: 1px solid rgba(210, 215, 223, 0.26);
			padding: 12px 24px;
			border-radius: 4px;
			color: rgb(210, 215, 223);
			cursor: pointer;
			margin-top: 20px;
			font-size: 16px;
			font-family: sans-serif;
			transition: all 0.3s ease;
			width: 300px;
		}

		.submit-btn:hover {
			background-color: rgb(45, 50, 57);
			border-color: rgb(210, 215, 223);
			transform: translateY(-2px);
			box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
		}

		.submit-btn:active {
			transform: translateY(0);
			box-shadow: none;
		}

		.text-input {
			width: 300px;
			padding: 8px 16px;
			background-color: rgb(35, 40, 47);
			border: 1px solid rgba(210, 215, 223, 0.26);
			border-radius: 4px;
			color: rgb(210, 215, 223);
			font-family: sans-serif;
			font-size: 14px;
			transition: all 0.3s ease;
			box-sizing: border-box;
		}

		.text-input:focus {
			outline: none;
			border-color: rgb(210, 215, 223);
			box-shadow: 0 0 5px rgba(210, 215, 223, 0.3);
		}

		.text-input::placeholder {
			color: rgba(210, 215, 223, 0.5);
		}

		.form-label {
			color: rgb(210, 215, 223);
			font-family: sans-serif;
			align-self: flex-start;
			margin-left: 50px;
			font-size: 14px;
		}

		.textarea-input {
			width: 300px;
			height: 100px;
			padding: 8px 16px;
			background-color: rgb(35, 40, 47);
			border: 1px solid rgba(210, 215, 223, 0.26);
			border-radius: 4px;
			color: rgb(210, 215, 223);
			font-family: sans-serif;
			font-size: 14px;
			transition: all 0.3s ease;
			box-sizing: border-box;
			resize: vertical;
		}

		.textarea-input:focus {
			outline: none;
			border-color: rgb(210, 215, 223);
			box-shadow: 0 0 5px rgba(210, 215, 223, 0.3);
		}
	</style>
</head>

<body style="background-color: rgb(35, 40, 47);">
    <div style="border: 1px solid rgba(210, 215, 223, 0.26); border-radius: 4px; margin: 100px auto; background-color: rgb(22, 27, 34); padding: 20px; max-width: 400px;">
        <h2 style="font-size: 25px; font-family: sans-serif; text-align: center; padding: 0px 0px 25px 0px; margin: 0px; color: rgb(210, 215, 223);">Contact Form</h2>
		<div><?=$_POST["firstName"] ?? "nothing turned in"?></div>
		<div><?=$_POST["lastName"] ?? "nothing turned in"?></div>
		<div><?=$_POST["email"] ?? "nothing turned in"?></div>
		<div><?=$_POST["phone"] ?? "nothing turned in"?></div>
    </div>
</body>

