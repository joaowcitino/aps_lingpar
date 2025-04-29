import re

class PrePro:
    @staticmethod
    def filter(code: str) -> str:
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
        code = re.sub(r'//.*', '', code)
        return code