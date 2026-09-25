module Main where

import Text.Parsec
import Text.Parsec.String (Parser)

data Expr
  = Num Int
  | Var String
  deriving (Show)

data Statement = Assign String Expr
  deriving (Show)

parseVarName :: Parser String
parseVarName = many1 letter

parseNumber :: Parser Expr
parseNumber = Num . read <$> many1 digit

-- parseVarExpr turns a parsed String into a Var expression
parseVarExpr :: Parser Expr
parseVarExpr = Var <$> parseVarName

-- Choice: An expression is either a Number OR a Variable
parseExpr :: Parser Expr
parseExpr = parseNumber <|> parseVarExpr

-- Helper: reads a string and automatically discards trailing spaces
symbol :: String -> Parser String
symbol s = string s <* spaces

-- 3. Our Assignment Parser
parseAssignment :: Parser Statement
parseAssignment =
  Assign
    <$> (symbol "let" *> parseVarName <* spaces)
    <*> (symbol "=" *> parseExpr)

-- 4. Execution
main :: IO ()
main = do
  let testCode = "let score = 100"

  -- 'parse' takes 3 arguments: the parser, a source name (for error messages), and the string
  case parse parseAssignment "" testCode of
    Left err -> putStrLn $ "Error: " ++ show err
    Right ast -> putStrLn $ "Success: " ++ show ast
