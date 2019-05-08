<?php
	/**
	 * 
	 * Database 
	 * https://standards.ieee.org/products-services/regauth/index.html?utm_source=Mainsite_CSE&utm_medium=CSE_Promotion&utm_term=oui&utm_campaign=Standards_Promotion-oui
	 * 
	 * CSV Registry,Assignment,Organization Name,Organization Address
	 * http://standards-oui.ieee.org/oui/oui.csv 3 bytes
	 * http://standards-oui.ieee.org/oui28/mam.csv 4 bytes
	 * http://standards-oui.ieee.org/oui36/oui36.csv 5 bytes
	 * 
	 * http://standards-oui.ieee.org/oui.txt
	 * Usage:
	 * cat 3 | php vendor.php 
	 * Files:
	 * vendor.txt
	 * 
	 * cat 1 | php vendor.php  > 4
	 * cat 3 | php vendor.php  > 4.3
	 * cat 4.3 | cut -f 2 | sort | uniq -c > 
	 */
	$vendorPrefix = array();
	loadVendors($vendorPrefix, 'ieee/oui.csv', 6);
	loadVendors($vendorPrefix, 'ieee/mam.csv', 8);
	loadVendors($vendorPrefix, 'ieee/oui36.csv', 10);
	// print_r($vendorPrefix);
	// exit(1);
	while ($l = trim(fgets(STDIN))) 
	{
  	print(getVendorName($vendorPrefix, $l) . "\n");
	}

	function loadVendors(&$out, $filename, $sz)
	{
		$f = fopen($filename, "r");
		$l = fgets($f); // skip header
		while ($p = fgetcsv($f, 1000, ",")) {
			$m = strtolower($p[1]);
			while (strlen($m) < $sz) { 
				$m = "0" . $m;
			}
			$out[$m] = trim($p[2]);
		}
		fclose($f);
	}

	function getVendorName(&$vendorPrefix, $macLine)
	{
		$prefix6 = substr($macLine, 0, 2) . substr($macLine, 3, 2) . substr($macLine, 6, 2);
		$prefix8 = $prefix6 . substr($macLine, 9, 2);
		$prefix10 = $prefix8 . substr($macLine, 12, 2);
		if (array_key_exists($prefix6, $vendorPrefix)) {
			$v = $vendorPrefix[$prefix6];
		} else {
			if (array_key_exists($prefix8, $vendorPrefix)) {
				$v = $vendorPrefix[$prefix8];
			} else {
				if (array_key_exists($prefix10, $vendorPrefix)) {
					$v = $vendorPrefix[$prefix10];
				} else {
					// $v = "NONAME $macLine";
					$v = "NONAME";
				}
			}
		}
		return $macLine . "\t" . $v;
	}
	
?>
