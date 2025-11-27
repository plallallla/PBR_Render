#include <glm/glm.hpp>
#include <variant>
#include <array>

enum class light_type
{
    point,
    spot,
    directional,
};

struct PointLight
{
    glm::vec3 position;
    std::array<float, 3> attenuation;
};

struct SpotLight
{
    glm::vec3 direction;
    glm::vec3 position;
    std::array<float, 2> cut_off;
};

struct DirectionalLight
{
    glm::vec3 direction;
};

struct Light
{
    light_type type;
    glm::vec3 irradiance;
    std::variant<std::monostate, PointLight, SpotLight, DirectionalLight> detail;
};

