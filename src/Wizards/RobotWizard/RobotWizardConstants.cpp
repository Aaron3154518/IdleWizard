#include "RobotWizardConstants.h"

#include <Components/FireballBase.h>
#include <Wizards/PowerWizard/PowerFireball.h>

namespace RobotWizard {
namespace Constants {
const std::vector<WizardId> UP_TARGETS{CRYSTAL, WIZARD, POWER_WIZARD,
                                       TIME_WIZARD};
const std::unordered_map<WizardId, ParameterSystem::StateParam> SYN_TARGETS = {
    {CRYSTAL, Params::get(Param::CrysSynBotActive)},
    {WIZARD, Params::get(Param::WizSynBotActive)},
    {TIME_WIZARD, Params::get(Param::TimeWizSynBotActive)},
};

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/robot_ss.png", 6, 100};
    return IMG;
}
const AnimationData& BOT_IMG() {
    const static AnimationData UP_BOT_IMG{"res/wizards/upgrade_bot_ss.png", 4,
                                          80};
    return UP_BOT_IMG;
}
const std::string& BOT_HAT_IMG(WizardId id) {
    const static std::string WIZ_HAT = "res/wizards/bot_wiz_hat.png",
                             CRYS_HAT = "res/wizards/bot_crys_hat.png",
                             TIME_WIZ_HAT = "res/wizards/bot_time_wiz_hat.png";

    switch (id) {
        case WIZARD:
            return WIZ_HAT;
        case CRYSTAL:
            return CRYS_HAT;
        case TIME_WIZARD:
            return TIME_WIZ_HAT;
    }

    throw std::runtime_error("SYN_BOT_IMG(): Invalid Wizard Id (" +
                             std::to_string(id) + ")");

    const static std::string EMPTY;
    return EMPTY;
}
RenderTextureCPtr BOT_FLOAT_IMG(WizardId id) {
    static float maxHover = 0;
    static RectShape fill(TRANSPARENT);
    static RenderAnimation bot;
    static std::unordered_map<WizardId, float> hovers;
    static std::unordered_map<WizardId, RenderData> hats;
    static std::unordered_map<WizardId, RenderTexturePtr> imgs;
    static std::unordered_map<WizardId, TextureBuilder> texs;
    static std::unordered_map<WizardId, TimerObservable::SubscriptionPtr>
        timerSubs;

    auto& img = imgs[id];

    if (!img) {
        std::mt19937 gen = std::mt19937(rand());
        std::uniform_real_distribution<float> rDist;
        hovers[id] = (rDist(gen) - .5) * M_PI;

        auto& hat = hats[id];
        auto& tex = texs[id];

        bot.set(BOT_IMG());
        hat.set(BOT_HAT_IMG(id));

        SDL_Point botDim = bot->getTextureDim();
        botDim = {botDim.x * 2, botDim.y * 2};
        SDL_Point hatDim = hat->getTextureDim();
        hatDim = {hatDim.x * 2, hatDim.y * 2};
        maxHover = (float)botDim.y / 4;
        int h = botDim.y + hatDim.y;

        tex = TextureBuilder(botDim.x, h);
        fill.set(Rect(0, 0, botDim.x, h));
        bot.setDest(Rect(0, h - botDim.y, botDim.x, botDim.y));
        Rect hatRect(0, 0, hatDim.x, hatDim.y);
        hatRect.setPosX(botDim.x / 2, Rect::Align::CENTER);
        hat.setDest(hatRect);

        img = std::make_shared<RenderTexture>(tex.getTexture());

        timerSubs[id] =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [id](Timer& t) {
                    auto& hover = hovers[id];
                    auto& hat = hats[id];
                    auto& tex = texs[id];

                    bot->nextFrame();

                    hover = fmodf(hover + M_PI * t.length / 600, 2 * M_PI);
                    Rect hatRect = hat.getDest();
                    hatRect.setPosY((sinf(hover) + 1) / 2 * maxHover,
                                    Rect::Align::TOP_LEFT);
                    hat.setDest(hatRect);

                    tex.draw(fill);
                    tex.draw(bot);
                    tex.draw(hat);

                    imgs[id]->update();

                    return true;
                },
                BOT_IMG());
    }

    return img;
}
const AnimationData& PORTAL_TOP() {
    const static AnimationData PORTAL_TOP{"res/wizards/portal_top.png", 6, 150};
    return PORTAL_TOP;
}
const AnimationData& PORTAL_BOT() {
    const static AnimationData PORTAL_BOT{"res/wizards/portal_bottom.png", 6,
                                          150};
    return PORTAL_BOT;
}

void setDefaults() {
    using WizardSystem::Event;

    Params params;

    // Values
    params[Param::Shards]->init(0, Event::ResetT2);
    params[Param::UpBotBaseCap]->init(100);
    params[Param::UpBotBaseRate]->init(0.05);

    // Upgrade costs
    params[Param::ShardPowerUpCost]->init(1);
    params[Param::WizCritUpCost]->init(Number(1, 7));
    params[Param::UpBotCapRateUpCost]->init(25);
    params[Param::NewCatUpsCost]->init(Number(5, 3));

    params[Param::UpBotCost]->init(5);
    params[Param::WizSynBotCost]->init(100);
    params[Param::CrysSynBotCost]->init(1000);
    params[Param::TimeWizSynBotCost]->init(Number(1, 5));

    // Bought upgrades
    params[Param::BoughtShardPowerUp]->init(false, Event::ResetT2);
    params[Param::BoughtWizCritUp]->init(false, Event::ResetT2);
    params[Param::BoughtUpBotCapRateUp]->init(false, Event::ResetT2);
    params[Param::BoughtNewCatUps]->init(false, Event::ResetT2);

    // Active bots
    params[Param::UpBotActive]->init(false, Event::ResetT2);
    params[Param::CrysSynBotActive]->init(false, Event::ResetT2);
    params[Param::WizSynBotActive]->init(false, Event::ResetT2);
    params[Param::TimeWizSynBotActive]->init(false, Event::ResetT2);
}
}  // namespace Constants
}  // namespace RobotWizard
