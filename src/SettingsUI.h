#include "Common.h"
#include "RendererRT.h"
#include "Overlay.h"

class MenuItem {
public:
	virtual std::string text() = 0;
	virtual void increase() = 0;
	virtual void decrease() = 0;
};

class FloatMenuItem : public MenuItem {
	std::string name;
	float& value;
	float changeStep;
public:
	FloatMenuItem(std::string name, float& value, float step = 0.1f) : name(name), value(value), changeStep(step) {};

	std::string text() {
		return string_format("%s: %.3f", name, value);
	}

	void increase() {
		value += changeStep;
	}

	void decrease() {
		value -= changeStep;
	}
};

class IntMenuItem : public MenuItem {
	std::string name;
	int& value;
	int changeStep;
public:
	IntMenuItem(std::string name, int& value, int step = 1) : name(name), value(value), changeStep(step) {};

	std::string text() {
		return string_format("%s: %d", name, value);
	}

	void increase() {
		value += changeStep;
	}

	void decrease() {
		value -= changeStep;
	}
};

class BoolMenuItem : public MenuItem {
	std::string name;
	bool& value;
public:
	BoolMenuItem(std::string name, bool& value) : name(name), value(value) {};

	std::string text() {
		return string_format("%s: %s", name, value ? "true" : "false");
	}

	void increase() {
		value = !value;
	}

	void decrease() {
		value = !value;
	}
};

class SettingsUI {
	std::vector<std::unique_ptr<MenuItem>> items = std::vector<std::unique_ptr<MenuItem>>(0);
	int position = 0;

public:
	SettingsUI() {};
	SettingsUI(std::vector<std::unique_ptr<MenuItem>> items) : items(std::move(items)) {};

	//SettingsUI(const SettingsUI&) = default;
	//SettingsUI& operator=(const SettingsUI&) = default;

	void handleKeyPress(SDL_Event event) {
		switch (event.key.keysym.sym) {
		case SDLK_DOWN:
			position += 1;
			if (position >= items.size()) position = 0;
			break;
		case SDLK_UP:
			position -= 1;
			if (position < 0) position = items.size() - 1;
			break;
		case SDLK_LEFT:
			items[position]->decrease();
			break;
		case SDLK_RIGHT:
			items[position]->increase();
			break;
		default:
			break;
		}
	}

	void drawOverlay(UIContext* ctx) {
		overlay::column(ctx, [&ctx, this]() {
			for (int i = 0; i < items.size(); i++) {
				overlay::text(ctx, items[i]->text(), i == position ? glm::vec3(0, 0, 255) : glm::vec3(255));
			}
		});
	}
};