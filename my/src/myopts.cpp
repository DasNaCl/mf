#include <myopts.hpp>

#include <iostream>
#include <cassert>

CmdOptions::CmdOptions(std::string name, std::string description)
  : name(name), description(description), opts()
{  }

CmdOptions::CmdOptionsAdder CmdOptions::add_options()
{
  return CmdOptionsAdder(*this);
}

void CmdOptions::add(std::string option, std::string description, CmdOptions::Value::Ptr val, std::string default_value)
{
  std::vector<std::string> short_names, long_names;
  
  std::stringstream ss(option);
  for(std::string str; std::getline(ss, str, ','); )
  {
    (str.size() > 1 ? long_names : short_names).push_back(str);
  }
  auto value = val->clone();
  value->default_value = default_value;

  opts.push_back({long_names, short_names, description, value});
}

CmdOptions::ValueMapping CmdOptions::parse(int argc, const char** argv)
{
  bool allows_dashdash = false;
  bool has_empty_arg = false;
  ValueMapping map;
  for(auto& opt : opts)
  {
    for(auto& n : opt.short_names)
    {
      map[n] = opt.value;
      if(n.empty())
        has_empty_arg = true;
      if(n == "-")
        allows_dashdash = true;
    }
    for(auto& n : opt.long_names)
      map[n] = opt.value;
  }
  for(int i = 1; i < argc; )
  {
    // argument must be prefixed with - or --
    // EXCEPT if there is an empty arg, which we then implicitly assume to be set
    std::string arg = argv[i];
    assert(!arg.empty());

    bool long_arg = false;
    if(arg[0] == '-')
    {
      if(arg[1] == '-' && arg.size() > 1)
      {
        if(!(arg[1] == '-' && arg[2] == '\0' && allows_dashdash))
        {
          long_arg = true;
          arg.erase(arg.begin());
          arg.erase(arg.begin());
        }
        else
        {
          long_arg = false;
          arg.erase(arg.begin()); // -- is - in opt.short_names
        }
      }
      else
      {
        arg.erase(arg.begin());
      }
    }
    else
    {
      if(has_empty_arg)
      {
        auto it = std::find_if(opts.begin(), opts.end(), [arg](const Option& opt)
                               { return std::find(opt.short_names.begin(), opt.short_names.end(), "") != opt.short_names.end(); });
        assert(it != opts.end());

        if(dynamic_cast<TaggedValue<std::vector<std::string>>*>(it->value.get()) != nullptr)
          --i;
        it->value->parse(i, argc, argv);
        continue;
      }
      else
      {
        handle_unrecognized_option(arg);
        continue;
      }
    }

    if(long_arg)
    {
      auto it = std::find_if(opts.begin(), opts.end(), [arg](const Option& opt)
                             { return std::find(opt.long_names.begin(), opt.long_names.end(), arg) != opt.long_names.end(); });
      if(it == opts.end())
      {
        handle_unrecognized_option(argv[i]);
        ++i;
      }
      else
      {
        it->value->parse(i, argc, argv);
      }
    }
    else
    {
      auto it = std::find_if(opts.begin(), opts.end(),
                [arg](const Option& opt)
                { return std::find_if(opt.short_names.begin(), opt.short_names.end(), [arg](const std::string& str)
                                                                          { return !str.empty() && arg.find(str) != std::string::npos; }) != opt.short_names.end(); });
      do
      {
        if(it == opts.end())
        {
          handle_unrecognized_option(argv[i]);
          ++i;
        }
        else
        {
          if(arg[0] == '-' && dynamic_cast<TaggedValue<std::vector<std::string>>*>(it->value.get()) != nullptr)
            --i;
          it->value->parse(i, argc, argv);
        }
        it = std::find_if(std::next(it), opts.end(),
                [arg](const Option& opt)
                { return std::find_if(opt.short_names.begin(), opt.short_names.end(), [arg](const std::string& str)
                                                                          { return !str.empty() && arg.find(str) != std::string::npos; }) != opt.short_names.end(); });
      } while(it != opts.end());
    }
  }
  return map;
}

void CmdOptions::print_help()
{
  std::cout << name << "  -  " << description << "\n"
            << "Usage:\n";
  for(auto& opt : opts)
  {
    std::cout << "\t";
    for(auto beg = opt.short_names.begin(), lbeg = opt.long_names.begin(); std::next(lbeg) != opt.long_names.end(); beg == opt.short_names.end() ? ++lbeg : ++beg)
      std::cout << *(beg == opt.short_names.end() ? lbeg : beg) << ",";

    std::cout << opt.long_names.back() << "\t" << opt.description << std::endl;
  }
}

void CmdOptions::handle_unrecognized_option(std::string opt)
{
  std::cout << "Unrecognized option: \"" << opt << "\"\n";
}

CmdOptions::CmdOptionsAdder::CmdOptionsAdder(CmdOptions& ref)
  : ref(ref)
{  }

CmdOptions::CmdOptionsAdder& CmdOptions::CmdOptionsAdder::operator()(std::string option, std::string description, CmdOptions::Value::Ptr value, std::string default_value)
{
  ref.add(option, description, value, default_value);

  return *this;
}

