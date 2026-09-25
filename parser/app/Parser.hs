module Parser where

import Syntax
import Text.Parsec
import Text.Parsec.String (Parser)

parseProgram :: Parser [ASTNode]
parseProgram = many (S <$> parseStruct <|> I <$> parseInterface) <* eof

-- Helpers

symbol :: String -> Parser String
symbol s = string s <* spaces

parseNumber :: Parser Float
parseNumber = (read <$> parseFloatString) <* spaces

parseFloatString :: Parser String
parseFloatString = (++) <$> many1 digit <*> option "" parseDecimal

parseDecimal :: Parser String
parseDecimal = (:) <$> char '.' <*> many1 digit

parseString :: Parser String
parseString = many1 (alphaNum <|> char '_') <* spaces

parseIdentifier :: Parser String
parseIdentifier = (:) <$> letter <*> parseTail

parseTail :: Parser String
parseTail = many (alphaNum <|> char '_') <* spaces

-- Shared

parseTypeMap :: Parser TypeMap
parseTypeMap = between (symbol "<") (symbol ">") (try (KV <$> (parseTypeArg <* symbol ",") <*> parseTypeArg) <|> (V <$> parseTypeArg))

-- Struct
-- 1, 2, 3, 4
parseStruct :: Parser Struct
parseStruct = Struct <$> parseStructName <*> parseExtends <*> parseImplements <*> parseConfigBlock

-- 1
parseStructName :: Parser StructName
parseStructName = symbol "struct" *> parseIdentifier

-- 2
parseExtends :: Parser Extends
parseExtends = Extends <$> (symbol "is" *> parseIdentifier) <*> parseTypeMap

parseTypeArg :: Parser String
parseTypeArg = many1 (noneOf "<,>") <* spaces

-- 3
parseImplementsList :: Parser [String]
parseImplementsList = symbol "implements" *> sepBy parseString (symbol ",")

parseImplements :: Parser (Maybe [String])
parseImplements = optionMaybe parseImplementsList

-- 4
parseConfigBlock :: Parser [ConfigOption]
parseConfigBlock = symbol "{" *> sepBy parseConfigOption (symbol ",") <* symbol "}"

parseConfigOption :: Parser ConfigOption
parseConfigOption = ConfigOption <$> (parseConfigKey <* symbol "=") <*> parseConfigValue

parseConfigKey :: Parser String
parseConfigKey = parseString

parseConfigValue :: Parser ConfigValue
parseConfigValue = (Num <$> parseNumber) <|> (Str <$> parseString)

--
-- Interface
--
parseInterface :: Parser Interface
parseInterface = Interface <$> parseInterfaceName <*> parseTypeMap <*> parseNamedInterface

parseInterfaceName :: Parser InterfaceName
parseInterfaceName = symbol "interface" *> parseIdentifier

parseNamedInterface :: Parser NamedInterface
parseNamedInterface = symbol "named" *> parseIdentifier
