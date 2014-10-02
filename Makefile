.PHONY: clean All

All:
	@echo "----------Building project:[ Chip8 - Debug ]----------"
	@$(MAKE) -f  "Chip8.mk"
clean:
	@echo "----------Cleaning project:[ Chip8 - Debug ]----------"
	@$(MAKE) -f  "Chip8.mk" clean
