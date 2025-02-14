<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Our Team</title>
   <style>
		body{
			background-color: rgb(35, 40, 47);
		}
		.container {
			max-width: 1200px;
			margin: 80px auto;
			padding: 100px 20px 400px 20px;
		}

		.header {
			text-align: center;
			color: rgb(210, 215, 223);
			margin-bottom: 50px;
		}

		.team-grid {
			display: grid;
			grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
			gap: 30px;
			margin-top: 40px;
		}

		.team-member {
			background-color: rgb(22, 27, 34);
			border: 1px solid rgba(210, 215, 223, 0.26);
			border-radius: 8px;
			padding: 20px;
			text-align: center;
			transition: transform 0.3s ease, box-shadow 0.3s ease;
		}

		.team-member:hover {
			transform: translateY(-5px);
			box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
		}


		.member-name {
			color: rgb(210, 215, 223);
			font-size: 24px;
			margin: 10px 0;
		}

		.member-role {
			color: rgba(210, 215, 223, 0.7);
			font-size: 18px;
			margin-bottom: 15px;
		}

		.member-bio {
			color: rgba(210, 215, 223, 0.9);
			font-size: 14px;
			line-height: 1.6;
			margin-bottom: 20px;
		}

		.social-links {
			display: flex;
			justify-content: center;
			gap: 15px;
			margin-top: 20px;
		}

		.social-link {
			color: rgb(210, 215, 223);
			text-decoration: none;
			padding: 8px 16px;
			border: 1px solid rgba(210, 215, 223, 0.26);
			border-radius: 4px;
			transition: all 0.3s ease;
		}

		.social-link:hover {
			background-color: rgba(210, 215, 223, 0.1);
			transform: translateY(-2px);
		}

		@media (max-width: 768px) {
			.team-grid {
				grid-template-columns: 1fr;
			}
		}

   </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1 style="font-size: 40px; margin-bottom: 10px;">Our Team</h1>
            <p style="font-size: 18px; color: rgba(210, 215, 223, 0.7);">Meet the amazing people behind our success</p>
        </div>
		<div class="team-grid">
		<?php 
			foreach ($members as $key => $value) {
		?>
				<div class="team-member">
					<h2 class="member-name"><?=$value["name"]?></h2>
					<div class="member-role"><?=$value["task"]?>.</div>
					<p class="member-bio">A computer science student at 1337 coding school</p>
					<div class="social-links">
						<a href="https://github.com/<?=$value["github"]?>" class="social-link">GitHub</a>
						<a href="https://linkedin.com/in/<?=$value["linkdin"]?>" class="social-link">LinkedIn</a>
					</div>
				</div>
		<? } ?>
        </div>
    </div>
</body>
</html>
