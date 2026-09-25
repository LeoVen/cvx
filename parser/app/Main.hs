module Main where

import Analyzer
import Parser
import Text.Parsec

main :: IO ()
main = do
    let file = "code.dsdef"
    fileContents <- readFile file

    case parse parseProgram file fileContents of
        Left err -> putStrLn $ "Error: " ++ show err
        Right ast -> do
            putStrLn $ "Success: " ++ show ast
            let errors = analyzeAll ast

            putStrLn $ "Errors: " ++ show errors
