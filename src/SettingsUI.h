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
public:
	FloatMenuItem(std::string name, float& value) : name(name), value(value) {};

	std::string text() {
		return string_format("%s: %.3f", name, value);
	}

	void increase() {
		value += 0.1f;
	}

	void decrease() {
		value -= 0.1f;
	}
};

class SettingsUI {
	Light& light;
	std::vector<std::unique_ptr<MenuItem>> items;
	int position = 0;

public:
	SettingsUI(Light& light) : light(light) {
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("light x", light.position.x)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("light y", light.position.y)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("light z", light.position.z)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("light intesity", light.intensity)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("diffusionFactor", light.diffusionFactor)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("inclineFactor", light.inclineFactor)));
		items.push_back(std::unique_ptr<FloatMenuItem>(new FloatMenuItem("specularFactor", light.specularFactor)));
	};

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

	void drawOverlay(UIContext& ctx) {
		overlay::column(ctx, [&ctx, this]() {
			for (int i = 0; i < items.size(); i++) {
				overlay::text(ctx, items[i]->text(), i == position ? glm::vec3(0, 0, 255) : glm::vec3(255));
			}
		});
	}
};