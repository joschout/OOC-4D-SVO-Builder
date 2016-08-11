#ifndef COLORTYPE_H
#define COLORTYPE_H

enum ColorType { COLOR_FROM_MODEL, COLOR_FIXED, COLOR_LINEAR, COLOR_NORMAL };

inline std::string color_type_to_string(ColorType type)
{
	switch(type)
	{
	case COLOR_FROM_MODEL: return "COLOR_FROM_MODEL";
	case COLOR_FIXED: return "COLOR_FIXED";
	case COLOR_LINEAR: return "COLOR_LINEAR";
	case COLOR_NORMAL: return "COLOR_NORMAL";
	}
	return {};
}

#endif
