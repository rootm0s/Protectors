<?php
$file = file_get_contents($argv[1]);
if(!file_exists("id.txt"))
	file_put_contents("id.txt", "0");
$lines = explode("\n", $file);
$enabled = false;
$randomdatac = 0;
$gen_id = intval(file_get_contents("id.txt")) + 1;
echo "bool nevertrue" . $gen_id . " = false;
void* useless_pointer" . $gen_id . " = nullptr;
";
foreach($lines as $line)
{
	echo $line . "\n";
	if(strpos($line, "//") !== false)
	{
		if(strpos($line, "//poli on") !== false)
			$enabled = true;
		else if(strpos($line, "//poli off") !== false)
			$enabled = false;
	}
	else if($enabled)
	{
		$iendpos = strrpos($line, ";");
		if(!($iendpos === false) && $iendpos == strlen($line) - 2)
		{
			$randomdatac++;
			echo "if(nevertrue" . $gen_id . ")
			{
				char randomdata" . $randomdatac . '[] = "' . substr(md5(uniqid(rand(), true)), 0, rand()%32) . '";
				useless_pointer' . $gen_id . ' = randomdata' . $randomdatac . ";
				__asm {";
				
				$n = rand()%255;
				for($i = 0; $i < $n; ++$i)
					printf("_emit 0x%02X;\r\n", rand() % 256);
				
			echo "}}";
		}
	}
}
file_put_contents("id.txt", $gen_id);
?>