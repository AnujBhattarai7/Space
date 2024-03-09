#pragma once

namespace Space
{
	struct Vec2
	{
		float x, y;

		Vec2(float X = 0.0f, float Y = 0.0f)
			:x(X), y(Y) {}

		const Vec2& ToInt() const
		{
			return Vec2(int(x), int(y));
		}
	};

	struct Vec3
	{
		float x, y, z;

		Vec3(float X = 0.0f, float Y = 0.0f, float Z = 0.0f)
			:x(X), y(Y), z(Z) {}

		const Vec3& ToInt() const
		{
			return Vec3(int(x), int(y), int(z));
		}
	};

	struct Vec4
	{
		float x, y, z, w;

		Vec4(float X = 0.0f, float Y = 0.0f, float Z = 0.0f, float W = 0.0f)
			:x(X), y(Y), z(Z),w(W) {}
		
		const Vec4& ToInt() const
		{
			return Vec4(int(x), int(y), int(z), int(w));
		}
	};
}
