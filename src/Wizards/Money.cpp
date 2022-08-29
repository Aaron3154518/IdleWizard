#include "Money.h"

namespace Money {
std::string GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    switch (param.mId) {
        case CRYSTAL:
            switch (param.mKey) {
                case CrystalParams::Magic:
                    return "res/wizards/crystal.png";
                case CrystalParams::Shards:
                    return "res/wizards/wizard.png";
            }
            break;
        case CATALYST:
            switch (param.mKey) {
                case CatalystParams::Magic:
                    return "res/wizards/catalyst.png";
            };
            break;
    };

    return "";
}
}  // namespace Money
