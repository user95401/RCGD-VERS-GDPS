#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
using namespace geode::prelude;
namespace fs {
	std::error_code err;
	using namespace std::filesystem;
	using namespace geode::utils::file;
}

#include <Geode/modify/LoadingLayer.hpp>
class $modify(LoadingLayerResourcesExt, LoadingLayer) {
	static void resourceSetup() {
		auto resources_dir = getMod()->getResourcesDir();

		auto tp = CCTexturePack();
		tp.m_paths = { string::pathToString(resources_dir).c_str() };
		tp.m_id = "resources"_spr;
		CCFileUtils::get()->addTexturePack(tp);

		for (const auto& entry : std::filesystem::recursive_directory_iterator(resources_dir, fs::err)) {
			if (!(entry.is_regular_file())) continue;
			if (!(entry.path().parent_path() == resources_dir)) continue;

			auto name = string::pathToString(entry.path().filename());

			if (string::containsAll(name, { "__unzip[", "].zip" })) {
				auto unzipPath = string::pathToString(entry.path().filename());
				unzipPath = string::replace(unzipPath, "__unzip[", "");
				unzipPath = string::replace(unzipPath, "].zip", "");
				unzipPath = string::replace(unzipPath, "..", "/");
				unzipPath = string::replace(unzipPath, "{game}", string::pathToString(dirs::getGeodeDir()));
				unzipPath = string::replace(unzipPath, "{save}", string::pathToString(dirs::getSaveDir()));
				unzipPath = string::replace(unzipPath, "{resources}", string::pathToString(getMod()->getResourcesDir()));

				auto forced = string::contains(unzipPath, "_forced");
				;;; unzipPath = string::replace(unzipPath, "_forced", "");

				fs::create_directories(unzipPath, fs::err);

				if (auto a = fs::Unzip::create(entry.path()); auto unzip = &a.unwrap()) {

					std::set<fs::path> unzipPaths;
					for (const auto& entry : unzip->getEntries()) unzipPaths.insert(unzipPath / entry);

					auto allFilesExtracted = true;
					for (const auto& entry : unzipPaths) if (not fs::exists(entry, fs::err)) {
						allFilesExtracted = false;
						break;
					};

					if (!allFilesExtracted or forced) {
						if (auto err = unzip->extractAllTo(unzipPath).err()) log::error("{}", err);
						else log::info("{} extracted to {}", entry.path(), unzipPath);
					};

				}
			}
		}
	}
	bool init(bool penis) {
		resourceSetup();
		return LoadingLayer::init(penis);
	}
};

#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayerResourcesExt, MenuLayer) {
	bool init() {
		auto deps = getMod()->getMetadata().getDependencies();
		for (const auto& dep : deps) {
			if (!Loader::get()->isModInstalled(dep.id)) game::restart();
			if (!Loader::get()->isModLoaded(dep.id)) {
				auto _ = Notification::create(
					" The " + dep.id + " isn't loaded!\n Enable it please..",
					NotificationIcon::Error, 0.f
				);
				_->show();
				auto menu = CCMenu::create();
				_->addChild(menu);
				menu->setPosition(CCPointZero);
				menu->addChild(CCMenuItemExt::createSpriteExtra(CCLayer::create(), [=](void*) {
					createQuickPopup(
						"Ready to restart?", "", "Restart", "Later", [=](void*, bool a) {
							if (!a) game::restart();
						}
					);
					geode::openInfoPopup(dep.id);
					}));
			};
		}
		return MenuLayer::init();
	}
};