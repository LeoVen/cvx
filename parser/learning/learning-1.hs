-- OR
parseBool = parseTrue </> parseFalse

-- 'do' notation
parseAssignment = do
  string "let"
  space
  varName <- parseVariable
  space
  char '='
  space
  value <- parseNumber
  return (varName, value)

-- Returns Either (Left is error, Right is value)

-- Applicative operators

-- <$> (Map)
-- This operator runs the parser on the right, and if it succeeds, it feeds the
-- extracted data into a standard function on the left.
--
-- Parse a word, and immediately calculate its length. If it parsed "let", it outputs 3
length <$> parseWord

-- <*> (Chain and Feed)
-- This strings multiple parsers together in sequence, feeding all of their outputs into the function on the far left.
--
-- Assume there is the following function makeTuple a b = (a, b)
-- The following means: parse a word, THEN parse a number, and feed both extracted pieces of data into makeTuple
makeTuple <$> parseWord <*> parseNumber

-- <* and *> (Keep and Discard)
-- discards the output of the parser the arrow is pointing away from
-- parseWord <* space - runs both, but discards space
-- space *> parseWord - runs both, but discards space
--
-- The following parses a string "(42)" and discards both parentheses
char '(' *> parseNumber <* char ')'

-- AST

-- Example of parsing an assignment operation (e.g. "let name = 100")

-- Assign is a data constructor
data Statement = Assign String Expr
  deriving (Show)

-- Assuming "symbol" is a parser helper that reads a specific string and automatically discards any trailing spaces
parseAssignment :: Parser Statement
parseAssignment =
  Assign
    <$> (symbol "let" *> parseVarName)
    <*> (symbol "=" *> parseExpr)

-- function :: Type Signature
--

-- : operator
--
-- Writing (:) 'a' "pple" does the eSo, wxact same thing as 'a' : "pple".
concat :: String
concat = (:) 'a' "pple"
