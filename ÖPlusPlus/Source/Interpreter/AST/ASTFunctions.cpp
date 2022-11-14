#include "../Functions.h"

Value Functions::printf(ARGS)
{
	Value formatted = format_string(args);
	std::cout << formatted.ToString() << "\n";

	return Value(ValueTypes::Void);
}

Value Functions::format_string(ARGS)
{
	//EnsureTypeOfArg(args, 0, ValueTypes::String);

	std::string formatted = args.at(0).GetString();

	if (args.size() == 1)
		return args.at(0);

	// Iterate all args
	for (int i = 1; i < args.size(); i++)
	{
		std::size_t charPos = formatted.find('$');
		if (charPos == std::string::npos) // Nothing to format
			return Value(formatted, ValueTypes::String);

		// Remove the '$'
		formatted.erase(charPos, 1);

		// Insert the thing at that position
		formatted = formatted.insert(charPos, args.at(i).ToString());
	}

	return Value(formatted, ValueTypes::String);
}