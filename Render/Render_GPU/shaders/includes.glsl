
vec2 box_intersects(const vec3 min_pos, const vec3 max_pos, const vec3 origin, const vec3 dir)
{
  vec3 safe_dir = sign(dir) * max(vec3(1e-9f), abs(dir));
  vec3 tMin = (min_pos - origin) / safe_dir;
  vec3 tMax = (max_pos - origin) / safe_dir;
  vec3 t1 = min(tMin, tMax);
  vec3 t2 = max(tMin, tMax);
  float tNear = max(t1.x, max(t1.y, t1.z));
  float tFar = min(t2.x, min(t2.y, t2.z));

  return vec2(tNear, tFar);
}

float sphere_sdf(vec3 P)
{
  return length(P) - 0.5;
}

vec3 get_normal(vec3 P)
{
  float EPS = 0.001;

  vec3 normal = vec3(sphere_sdf(P + vec3(EPS, 0, 0)) - sphere_sdf(P + vec3(-EPS, 0, 0)) / (2 * EPS), 
                    sphere_sdf(P + vec3(0, EPS, 0)) - sphere_sdf(P + vec3(0, -EPS, 0)) / (2 * EPS), 
                    sphere_sdf(P + vec3(0, 0, EPS)) - sphere_sdf(P + vec3(0, 0, -EPS)) / (2 * EPS));
  normal = normalize(normal);
  return normal;
}