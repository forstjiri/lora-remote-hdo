
tx:
	@echo "Building for TX environment"
	pio run -t upload -t monitor -e tx
	@echo "Building for TX environment done"

rx:
	@echo "Building for RX environment"
	pio run -t upload -t monitor -e rx
	@echo "Building for RX environment done"