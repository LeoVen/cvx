module Analyzer where

import Data (allInterfaces, allStructs)
import Syntax

analyzeAll :: [ASTNode] -> [String]
analyzeAll nodes =
    let
        customInterfaces = [customName | I (Interface _ _ customName) <- nodes]
        structs = [s | S s <- nodes]
        interfaces = [i | I i <- nodes]

        env = setEnvironment customInterfaces

        errors = validatePrelude env structs interfaces
        userDefined = [err | custom <- customInterfaces, err <- validateUserDefinedInterface env custom]

        finalResult = errors ++ userDefined
     in
        finalResult

data Environment = Environment
    { knownStructs :: [String],
      knownInterfaces :: [String],
      newInterfaces :: [String]
    }
    deriving (Show)

setEnvironment :: [String] -> Environment
setEnvironment customInterfaces =
    Environment
        { knownStructs = allStructs,
          knownInterfaces = allInterfaces,
          newInterfaces = customInterfaces
        }

-- Validate prelude structs and interfaces
validatePrelude :: Environment -> [Struct] -> [Interface] -> [String]
validatePrelude env structs interfaces = [err | s <- structs, err <- validateStruct env s] ++ [err | i <- interfaces, err <- validateInterface env i]

validateStruct :: Environment -> Struct -> [String]
validateStruct env s = checkStruct env s ++ checkImplements env s

checkStruct :: Environment -> Struct -> [String]
checkStruct env (Struct _ (Extends extendsName _) _ _) = ["Unknown extends struct " ++ extendsName | extendsName `notElem` knownStructs env]

checkImplements :: Environment -> Struct -> [String]
checkImplements env (Struct _ _ impl _) =
    case impl of
        Nothing -> []
        Just interfaces -> ["Unknown interface " ++ interface | interface <- interfaces, interface `notElem` (knownInterfaces env ++ newInterfaces env)]

validateInterface :: Environment -> Interface -> [String]
validateInterface = checkInterface

checkInterface :: Environment -> Interface -> [String]
checkInterface env (Interface preludeInterface _ _) = ["Unknown base interface " ++ preludeInterface | preludeInterface `notElem` knownInterfaces env]

-- Validate user defined interfaces

validateUserDefinedInterface :: Environment -> NamedInterface -> [String]
validateUserDefinedInterface env namedInterface
    | namedInterface `notElem` newInterfaces env =
        [possibleValuesError (newInterfaces env) ("Undefined interface " ++ namedInterface)]
    | otherwise = []

possibleValuesError :: [String] -> String -> String
possibleValuesError possible msg = msg ++ " ---- Possible values are: " ++ show possible
