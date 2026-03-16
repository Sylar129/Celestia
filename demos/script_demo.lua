-- script_demo.lua
-- Demonstrates a Lua script loaded from disk by the C++ host.

local function factorial(n)
  if n <= 1 then return 1 end
  return n * factorial(n - 1)
end

local function greet(name)
  print(string.format("Hello from Lua script, %s!", name))
end

greet("Celestia")

for i = 1, 6 do
  print(string.format("  factorial(%d) = %d", i, factorial(i)))
end

-- Return a summary table so the C++ host can read results.
return {
  author  = "Celestia",
  version = "1.0",
  result  = factorial(6),
}
