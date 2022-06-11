my $first = 0;
while(<>) {
	if (/^##/ && !$first) {
		$first = 1;
		next;
	}
	if (/^##/ && $first) {
		exit;
	}
	print;
}
