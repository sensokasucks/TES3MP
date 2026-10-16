-- Minimal JSON encoder/decoder for the multiplayer event channel.
-- Tables with keys 1..n encode as arrays, other tables as objects; null decodes to nil.

local json = {}

local escapes = { ['"'] = '\\"', ['\\'] = '\\\\', ['\b'] = '\\b', ['\f'] = '\\f',
    ['\n'] = '\\n', ['\r'] = '\\r', ['\t'] = '\\t' }

local function encodeString(s)
    return '"' .. s:gsub('[%c"\\]', function(c)
        return escapes[c] or string.format('\\u%04x', c:byte())
    end) .. '"'
end

local function isArray(t)
    local count = 0
    for k in pairs(t) do
        if type(k) ~= 'number' or k < 1 or k % 1 ~= 0 then return false end
        count = count + 1
    end
    for i = 1, count do
        if t[i] == nil then return false end
    end
    return true, count
end

local function encodeValue(value, depth)
    local kind = type(value)
    if kind == 'nil' then
        return 'null'
    elseif kind == 'boolean' then
        return value and 'true' or 'false'
    elseif kind == 'number' then
        if value ~= value or value == math.huge or value == -math.huge then return 'null' end
        if value % 1 == 0 and math.abs(value) < 2^53 then return string.format('%d', value) end
        return string.format('%.14g', value)
    elseif kind == 'string' then
        return encodeString(value)
    elseif kind == 'table' then
        if depth > 32 then error('json: table nested too deeply') end
        local array, count = isArray(value)
        local parts = {}
        if array and count > 0 then
            for i = 1, count do parts[i] = encodeValue(value[i], depth + 1) end
            return '[' .. table.concat(parts, ',') .. ']'
        end
        for k, v in pairs(value) do
            if type(k) ~= 'string' and type(k) ~= 'number' then error('json: unsupported key type ' .. type(k)) end
            parts[#parts + 1] = encodeString(tostring(k)) .. ':' .. encodeValue(v, depth + 1)
        end
        return '{' .. table.concat(parts, ',') .. '}'
    end
    error('json: cannot encode a ' .. kind)
end

function json.encode(value)
    return encodeValue(value, 0)
end

local function decodeError(str, pos, message)
    error(string.format('json: %s at position %d', message, pos))
end

local function skipWhitespace(str, pos)
    return str:find('[^ \t\r\n]', pos) or #str + 1
end

local function utf8Char(code)
    if code < 0x80 then return string.char(code) end
    if code < 0x800 then return string.char(0xC0 + math.floor(code / 0x40), 0x80 + code % 0x40) end
    if code < 0x10000 then
        return string.char(0xE0 + math.floor(code / 0x1000), 0x80 + math.floor(code / 0x40) % 0x40, 0x80 + code % 0x40)
    end
    return string.char(0xF0 + math.floor(code / 0x40000), 0x80 + math.floor(code / 0x1000) % 0x40,
        0x80 + math.floor(code / 0x40) % 0x40, 0x80 + code % 0x40)
end

local decodeValue

local function decodeString(str, pos)
    local parts = {}
    local i = pos + 1
    while true do
        local c = str:sub(i, i)
        if c == '' then decodeError(str, i, 'unterminated string') end
        if c == '"' then return table.concat(parts), i + 1 end
        if c == '\\' then
            local e = str:sub(i + 1, i + 1)
            local simple = { ['"'] = '"', ['\\'] = '\\', ['/'] = '/', b = '\b', f = '\f', n = '\n', r = '\r', t = '\t' }
            if simple[e] then
                parts[#parts + 1] = simple[e]
                i = i + 2
            elseif e == 'u' then
                local hex = str:sub(i + 2, i + 5)
                local code = tonumber(hex, 16)
                if not code or #hex ~= 4 then decodeError(str, i, 'bad unicode escape') end
                i = i + 6
                if code >= 0xD800 and code <= 0xDBFF and str:sub(i, i + 1) == '\\u' then
                    local low = tonumber(str:sub(i + 2, i + 5), 16)
                    if low and low >= 0xDC00 and low <= 0xDFFF then
                        code = 0x10000 + (code - 0xD800) * 0x400 + (low - 0xDC00)
                        i = i + 6
                    end
                end
                parts[#parts + 1] = utf8Char(code)
            else
                decodeError(str, i, 'bad escape')
            end
        else
            local stop = str:find('["\\]', i) or #str + 1
            parts[#parts + 1] = str:sub(i, stop - 1)
            i = stop
        end
    end
end

function decodeValue(str, pos, depth)
    if depth > 32 then decodeError(str, pos, 'nested too deeply') end
    pos = skipWhitespace(str, pos)
    local c = str:sub(pos, pos)
    if c == '{' then
        local result = {}
        pos = skipWhitespace(str, pos + 1)
        if str:sub(pos, pos) == '}' then return result, pos + 1 end
        while true do
            pos = skipWhitespace(str, pos)
            if str:sub(pos, pos) ~= '"' then decodeError(str, pos, 'expected a key') end
            local key
            key, pos = decodeString(str, pos)
            pos = skipWhitespace(str, pos)
            if str:sub(pos, pos) ~= ':' then decodeError(str, pos, "expected ':'") end
            result[key], pos = decodeValue(str, pos + 1, depth + 1)
            pos = skipWhitespace(str, pos)
            local sep = str:sub(pos, pos)
            if sep == '}' then return result, pos + 1 end
            if sep ~= ',' then decodeError(str, pos, "expected ',' or '}'") end
            pos = pos + 1
        end
    elseif c == '[' then
        local result = {}
        pos = skipWhitespace(str, pos + 1)
        if str:sub(pos, pos) == ']' then return result, pos + 1 end
        local n = 0
        while true do
            n = n + 1
            result[n], pos = decodeValue(str, pos, depth + 1)
            pos = skipWhitespace(str, pos)
            local sep = str:sub(pos, pos)
            if sep == ']' then return result, pos + 1 end
            if sep ~= ',' then decodeError(str, pos, "expected ',' or ']'") end
            pos = pos + 1
        end
    elseif c == '"' then
        return decodeString(str, pos)
    elseif str:sub(pos, pos + 3) == 'true' then
        return true, pos + 4
    elseif str:sub(pos, pos + 4) == 'false' then
        return false, pos + 5
    elseif str:sub(pos, pos + 3) == 'null' then
        return nil, pos + 4
    end
    local number = str:match('^-?%d+%.?%d*[eE]?[-+]?%d*', pos)
    if number and tonumber(number) then
        return tonumber(number), pos + #number
    end
    decodeError(str, pos, 'unexpected character')
end

function json.decode(str)
    if type(str) ~= 'string' then error('json: expected a string') end
    local value, pos = decodeValue(str, 1, 0)
    pos = skipWhitespace(str, pos)
    if pos <= #str then decodeError(str, pos, 'trailing characters') end
    return value
end

return json
