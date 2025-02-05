<?php 

function getData()
{
    static $data = [
		"leader"=> [
			"name" => "tarik amehri",
			"github" => "tarikkudesu",
			"linkdin" => "tarik-amehri",
			"task" => "Core builder"
		],
		"member1"=>[
			"name" => "otman oulcaid",
			"github" => "otmanoulcaid",
            "linkdin" => "otmanoulcaid",
			"task" => "CGI Manager"
        ],
		"member2"=>[
			"name" => "omar ghazi",
			"github" => "Om7gh",
            "linkdin" => "oamr-ghazi0",
			"task" => "Response builder"
        ]
    ];

    return $data;
}

function getAll()
{
    return getData();
}

function getOne($fetch)
{
    $data = getData();

    foreach ($data as $key => $value)
        if (strcmp($key, $fetch) == 0)
            return [$key => $value];
    return ["erreur" => $fetch." not found"];
}