#ifndef slic3r_GUI_MD3Tokens_hpp_
#define slic3r_GUI_MD3Tokens_hpp_

#include <wx/colour.h>

namespace MD3 {

enum class Role
{
    Surface,
    SurfaceDim,
    SurfaceBright,
    SurfaceContainerLowest,
    SurfaceContainerLow,
    SurfaceContainer,
    SurfaceContainerHigh,
    SurfaceContainerHighest,
    OnSurface,
    OnSurfaceVariant,
    Outline,
    OutlineVariant,
    Primary,
    OnPrimary,
    PrimaryContainer,
    OnPrimaryContainer,
    SecondaryContainer,
    OnSecondaryContainer,
    Error,
    ErrorContainer,
    InverseSurface,
    InverseOn,
};

// Workspaces share the same neutral surfaces, type and error roles, while
// their primary accents communicate the current task context.  Keeping the
// schemes here prevents Preview/Device controls from falling back to legacy
// brand-green literals in individual wxWidgets and ImGui implementations.
enum class ColorScheme
{
    Brand,
    Preview,
    Device,
};

namespace Light {

inline const wxColour surface{"#faf8fd"};
inline const wxColour surfaceDim{"#dad9e0"};
inline const wxColour surfaceBright{"#faf8fd"};
inline const wxColour scLowest{"#ffffff"};
inline const wxColour scLow{"#f4f2f9"};
inline const wxColour sc{"#eeedf3"};
inline const wxColour scHigh{"#e8e7ee"};
inline const wxColour scHighest{"#e2e1e9"};
inline const wxColour onSurface{"#1a1b1f"};
inline const wxColour onSurfaceVariant{"#44464e"};
inline const wxColour outline{"#75777f"};
inline const wxColour outlineVariant{"#c5c6d0"};
inline const wxColour primary{"#146c2e"};
inline const wxColour onPrimary{"#ffffff"};
inline const wxColour primaryContainer{"#a6f4b8"};
inline const wxColour onPrimaryContainer{"#00210c"};
inline const wxColour secondaryContainer{"#d7e8d9"};
inline const wxColour onSecondaryContainer{"#0e1f13"};
inline const wxColour error{"#ba1a1a"};
inline const wxColour errorContainer{"#ffdad6"};
inline const wxColour inverseSurface{"#2f3036"};
inline const wxColour inverseOn{"#f1f0f7"};

} // namespace Light

namespace Dark {

inline const wxColour surface{"#1b1c21"};
inline const wxColour surfaceDim{"#161619"};
inline const wxColour surfaceBright{"#3b3c43"};
inline const wxColour scLowest{"#131317"};
inline const wxColour scLow{"#202127"};
inline const wxColour sc{"#25262b"};
inline const wxColour scHigh{"#2f3036"};
inline const wxColour scHighest{"#393a41"};
inline const wxColour onSurface{"#e8e7ee"};
inline const wxColour onSurfaceVariant{"#cdced8"};
inline const wxColour outline{"#94959f"};
inline const wxColour outlineVariant{"#4a4c54"};
inline const wxColour primary{"#8bd89b"};
inline const wxColour onPrimary{"#00391a"};
inline const wxColour primaryContainer{"#095228"};
inline const wxColour onPrimaryContainer{"#a6f4b8"};
inline const wxColour secondaryContainer{"#2b3a2f"};
inline const wxColour onSecondaryContainer{"#cfe9d3"};
inline const wxColour error{"#ffb4ab"};
inline const wxColour errorContainer{"#93000a"};
inline const wxColour inverseSurface{"#e3e2e9"};
inline const wxColour inverseOn{"#2f3036"};

} // namespace Dark

namespace Preview {

// #7c5cff is the approved Preview seed. The dark role is lifted for contrast
// against the shared dark surfaces, as prescribed by Material color roles.
inline const wxColour seed{"#7c5cff"};
inline const wxColour primaryLight{"#7050e8"};
inline const wxColour onPrimaryLight{"#ffffff"};
inline const wxColour primaryContainerLight{"#e8ddff"};
inline const wxColour onPrimaryContainerLight{"#23005c"};
inline const wxColour secondaryContainerLight{"#e5dff3"};
inline const wxColour onSecondaryContainerLight{"#211a2d"};
inline const wxColour primaryDark{"#ad98ff"};
inline const wxColour onPrimaryDark{"#2b006d"};
inline const wxColour primaryContainerDark{"#563bc2"};
inline const wxColour onPrimaryContainerDark{"#e8ddff"};
inline const wxColour secondaryContainerDark{"#494253"};
inline const wxColour onSecondaryContainerDark{"#e8dff5"};

} // namespace Preview

namespace Device {

// #14b8a6 is the approved Device seed. Accessible role tones are used for
// text-bearing controls in light and dark mode.
inline const wxColour seed{"#14b8a6"};
inline const wxColour primaryLight{"#0f766e"};
inline const wxColour onPrimaryLight{"#ffffff"};
inline const wxColour primaryContainerLight{"#9cf2e7"};
inline const wxColour onPrimaryContainerLight{"#00201d"};
inline const wxColour secondaryContainerLight{"#cce8e3"};
inline const wxColour onSecondaryContainerLight{"#08201d"};
inline const wxColour primaryDark{"#5eead4"};
inline const wxColour onPrimaryDark{"#003731"};
inline const wxColour primaryContainerDark{"#005047"};
inline const wxColour onPrimaryContainerDark{"#83f5e3"};
inline const wxColour secondaryContainerDark{"#304a46"};
inline const wxColour onSecondaryContainerDark{"#cce8e3"};

} // namespace Device

// Resolve by semantic role instead of by light-mode RGB value. Several MD3
// roles deliberately share a light value but diverge in dark mode (for
// example surface and surfaceBright), so a colour-to-colour lookup cannot
// preserve their meaning.
inline const wxColour &resolve(Role role, bool dark)
{
    if (dark) {
        switch (role) {
        case Role::Surface: return Dark::surface;
        case Role::SurfaceDim: return Dark::surfaceDim;
        case Role::SurfaceBright: return Dark::surfaceBright;
        case Role::SurfaceContainerLowest: return Dark::scLowest;
        case Role::SurfaceContainerLow: return Dark::scLow;
        case Role::SurfaceContainer: return Dark::sc;
        case Role::SurfaceContainerHigh: return Dark::scHigh;
        case Role::SurfaceContainerHighest: return Dark::scHighest;
        case Role::OnSurface: return Dark::onSurface;
        case Role::OnSurfaceVariant: return Dark::onSurfaceVariant;
        case Role::Outline: return Dark::outline;
        case Role::OutlineVariant: return Dark::outlineVariant;
        case Role::Primary: return Dark::primary;
        case Role::OnPrimary: return Dark::onPrimary;
        case Role::PrimaryContainer: return Dark::primaryContainer;
        case Role::OnPrimaryContainer: return Dark::onPrimaryContainer;
        case Role::SecondaryContainer: return Dark::secondaryContainer;
        case Role::OnSecondaryContainer: return Dark::onSecondaryContainer;
        case Role::Error: return Dark::error;
        case Role::ErrorContainer: return Dark::errorContainer;
        case Role::InverseSurface: return Dark::inverseSurface;
        case Role::InverseOn: return Dark::inverseOn;
        }
    }

    switch (role) {
    case Role::Surface: return Light::surface;
    case Role::SurfaceDim: return Light::surfaceDim;
    case Role::SurfaceBright: return Light::surfaceBright;
    case Role::SurfaceContainerLowest: return Light::scLowest;
    case Role::SurfaceContainerLow: return Light::scLow;
    case Role::SurfaceContainer: return Light::sc;
    case Role::SurfaceContainerHigh: return Light::scHigh;
    case Role::SurfaceContainerHighest: return Light::scHighest;
    case Role::OnSurface: return Light::onSurface;
    case Role::OnSurfaceVariant: return Light::onSurfaceVariant;
    case Role::Outline: return Light::outline;
    case Role::OutlineVariant: return Light::outlineVariant;
    case Role::Primary: return Light::primary;
    case Role::OnPrimary: return Light::onPrimary;
    case Role::PrimaryContainer: return Light::primaryContainer;
    case Role::OnPrimaryContainer: return Light::onPrimaryContainer;
    case Role::SecondaryContainer: return Light::secondaryContainer;
    case Role::OnSecondaryContainer: return Light::onSecondaryContainer;
    case Role::Error: return Light::error;
    case Role::ErrorContainer: return Light::errorContainer;
    case Role::InverseSurface: return Light::inverseSurface;
    case Role::InverseOn: return Light::inverseOn;
    }

    return Light::surface;
}

inline const wxColour &resolve(Role role, bool dark, ColorScheme scheme)
{
    if (scheme == ColorScheme::Brand)
        return resolve(role, dark);

    if (scheme == ColorScheme::Preview) {
        if (dark) {
            switch (role) {
            case Role::Primary: return Preview::primaryDark;
            case Role::OnPrimary: return Preview::onPrimaryDark;
            case Role::PrimaryContainer: return Preview::primaryContainerDark;
            case Role::OnPrimaryContainer: return Preview::onPrimaryContainerDark;
            case Role::SecondaryContainer: return Preview::secondaryContainerDark;
            case Role::OnSecondaryContainer: return Preview::onSecondaryContainerDark;
            default: return resolve(role, true);
            }
        }
        switch (role) {
        case Role::Primary: return Preview::primaryLight;
        case Role::OnPrimary: return Preview::onPrimaryLight;
        case Role::PrimaryContainer: return Preview::primaryContainerLight;
        case Role::OnPrimaryContainer: return Preview::onPrimaryContainerLight;
        case Role::SecondaryContainer: return Preview::secondaryContainerLight;
        case Role::OnSecondaryContainer: return Preview::onSecondaryContainerLight;
        default: return resolve(role, false);
        }
    }

    if (dark) {
        switch (role) {
        case Role::Primary: return Device::primaryDark;
        case Role::OnPrimary: return Device::onPrimaryDark;
        case Role::PrimaryContainer: return Device::primaryContainerDark;
        case Role::OnPrimaryContainer: return Device::onPrimaryContainerDark;
        case Role::SecondaryContainer: return Device::secondaryContainerDark;
        case Role::OnSecondaryContainer: return Device::onSecondaryContainerDark;
        default: return resolve(role, true);
        }
    }
    switch (role) {
    case Role::Primary: return Device::primaryLight;
    case Role::OnPrimary: return Device::onPrimaryLight;
    case Role::PrimaryContainer: return Device::primaryContainerLight;
    case Role::OnPrimaryContainer: return Device::onPrimaryContainerLight;
    case Role::SecondaryContainer: return Device::secondaryContainerLight;
    case Role::OnSecondaryContainer: return Device::onSecondaryContainerLight;
    default: return resolve(role, false);
    }
}

struct DensityMetrics
{
    int gap;
    int padding;
    int row_height;
    int font_size;
    int rail_width;
    int sidebar_width;
    int radius;
    int small_radius;
};

namespace Metrics {

inline constexpr DensityMetrics comfortable{12, 16, 40, 14, 60, 344, 16, 10};
inline constexpr DensityMetrics compact{7, 10, 32, 13, 50, 312, 12, 8};

inline constexpr int top_bar_height         = 46;
inline constexpr int navigation_bar_height  = 52;
inline constexpr int prepare_actions_height = 66;
inline constexpr int preview_timeline_height = 58;

} // namespace Metrics

} // namespace MD3

#endif // slic3r_GUI_MD3Tokens_hpp_
